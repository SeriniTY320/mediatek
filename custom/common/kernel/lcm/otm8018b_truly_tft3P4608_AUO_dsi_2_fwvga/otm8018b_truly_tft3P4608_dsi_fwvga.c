
#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_pmic.h>
	
	#define Lcd_Log printf
#else
    #include <linux/string.h>
	#include <linux/kernel.h>
	#include <mach/mt_gpio.h>
	#include <mach/mt_pm_ldo.h>
	
	#define Lcd_Log printk
#endif
#include "lcm_drv.h" 
 
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
 
#define FRAME_WIDTH                                        (480)
#define FRAME_HEIGHT                                       (854) 
 
#define REGFLAG_DELAY                                       0XFD
#define REGFLAG_END_OF_TABLE                                0xFE   // END OF REGISTERS MARKER
 
#define LCM_DSI_CMD_MODE                                   0
#define LCM_ID                                      0x8018
#define GPIO_LCD_ID_PIN GPIO_LCM_ID_PIN 
 
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
 
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)   lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)      lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                     lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                 lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                           lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                      lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)       
 
static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting1[] = {
   
   /*
   Note :
 
   Data ID will depends on the following rule.
   
       count of parameters > 1 => Data ID = 0x39
       count of parameters = 1 => Data ID = 0x15
       count of parameters = 0 => Data ID = 0x05
 
   Structure Format :
 
   {DCS command, count of parameters, {parameter list}}
   {REGFLAG_DELAY, milliseconds of time, {}},
 
   ...
 
   Setting ending by predefined flag
   
   {REGFLAG_END_OF_TABLE, 0x00, {}}
   */
  {0x00,    1,    {0x00}}, 		  // shift address	
	{0xff,    3,    {0x80,0x09,0x01}},        // enable EXTC
	{0x00,    1,    {0x80}},  		  // shift address
	{0xff,    2,    {0x80,0x09}}, 	  // enable Orise mode
	
	
	{0x00,    1,    {0x80}},  // shift address
	{0xF5,    12,   {0x01,0x18,0x02,0x18,0x10,0x18,0x02,0x18,0x0E,0x18,0x0F,0x20}},
    
 
	{0x00,    1,    {0x90}},  // shift address
	{0xF5,    10,   {0x02,0x18,0x08,0x18,0x06,0x18,0x0D,0x18,0x0B,0x18}},
    

	{0x00,    1,    {0xA0}},  // shift address
	{0xF5,    8,    {0x10,0x18,0x01,0x18,0x14,0x18,0x14,0x18}},
    

	{0x00,    1,    {0xB0}},  // shift address
	{0xF5,    12,   {0x14,0x18,0x12,0x18,0x13,0x18,0x11,0x18,0x13,0x18,0x00,0x00}},
    

	{0x00,    1,    {0x80}},           // pump
	{0xc0,    5,    {0x00,0x58,0x00,0x14,0x16}},

#if 1 //for S19D improve flicker
	{0x00,    1,    {0xB4}},  		  // Column Inversion
	{0xc0,    1,    {0x00}}, //0x50 maliejun modify
#else	
	{0x00,    1,    {0xB4}},  		  // Column Inversion
	{0xc0,    1,    {0x50}},
#endif	

	{0x00,    1,    {0x82}},           //  REG-Pump23 AVEE VCL
	{0xc5,    1,    {0xA3}},

	{0x00,    1,    {0x90}},           // pump
	{0xc5,    2,    {0xD6,0x76}},

	{0x00,    1,    {0x00}},          // GVDD 4.75V NGVDD 4.75V
	{0xD8,    2,    {0xAF,0xAF}},

	{0x00,    1,    {0x00}},          // VCOM
	{0xD9,    1,    {0x83}},          // 85

	{0x00,    1,    {0x81}},
	{0xC1,    1,    {0x77}},

	{0x00,    1,    {0xA1}},
	{0xC1,    1,    {0x08}},

	{0x00,    1,    {0xA3}},
	{0xC0,    1,    {0x1B}},
           
	{0x00,    1,    {0x81}},
	{0xC4,    1,    {0x83}},
	         
	{0x00,    1,    {0x92}},
	{0xC5,    1,    {0x01}},
           
	{0x00,    1,    {0xB1}},          // VDD_18V=1.6V GVDD test on
	{0xc5,    1,    {0xA9}},

//----------845*480----------------//
	{0x00,    1,    {0x90}},
	{0xB3,    1,    {0x02}},
	{0x00,    1,    {0x92}},
	{0xB3,    1,    {0x45}},
           
	{0x00,    1,    {0x40}},
	{0xB3,    1,    {0x45}},

//C09x : mck_shift1/mck_shift2/mck_shift3
	{0x00,    1,    {0x90}},
	{0xc0,    6,    {0x00,0x44,0x00,0x00,0x00,0x03}},	

//C0Ax : hs_shift/vs_shift
	{0x00,    1,    {0xa6}},
	{0xc1,    3,    {0x00,0x00,0x00}},

//--------------------------------------------------------------------------------
//	Initial Setting 2 < tcon_goa_wave >-->GOA Timing Setting
//--------------------------------------------------------------------------------
//CE8x : vst1, vst2, vst3, vst4
	{0x00,    1,    {0x80}}, // shift address
	{0xce,    6,    {0x85,0x03,0x00,0x84,0x03,0x00}},
//CE9x : 
	{0x00,    1,    {0x90}}, // shift address
	{0xce,    6,    {0x33,0x5C,0x00,0x33,0x5D,0x00}},
//CEAx : clka1, clka2
	{0x00,    1,    {0xa0}}, // shift address
	{0xCE,    14,   {0x38,0x03,0x03,0x56,0x00,0x00,0x00,0x38,0x02,0x03,0x57,0x00,0x00,0x00}},
//CEBx : clka3, clka4
	{0x00,    1,    {0xb0}}, // shift address
	{0xCE,    14,   {0x38,0x01,0x03,0x58,0x00,0x00,0x00,0x38,0x00,0x03,0x59,0x00,0x00,0x00}},
//CEBx : clkb1, clkb2
	{0x00,    1,    {0xc0}}, // shift address
	{0xCE,    14,   {0x30,0x00,0x03,0x5A,0x00,0x00,0x00,0x30,0x01,0x03,0x5B,0x00,0x00,0x00}},
//CEBx : clkb3, clkb4
	{0x00,    1,    {0xd0}}, // shift address
	{0xCE,    14,   {0x30,0x02,0x03,0x5C,0x00,0x00,0x00,0x30,0x03,0x03,0x5D,0x00,0x00,0x00}},


//CFCx :
	{0x00,    1,    {0xc7}}, // shift address
	{0xCF,    1,    {0x00}},

//--------------------------------------------------------------------------------
//	Initial Setting 3 < Panel setting >-->Output PAD Assign Setting
//--------------------------------------------------------------------------------
//CBCx
	{0x00,    1,    {0xc0}}, // shift address
	{0xCB,    15,   {0x00,0x00,0x00,0x00,0x54,0x54,0x54,0x54,0x00,0x54,0x00,0x54,0x00,0x00,0x00}},
//CBDx
	{0x00,    1,    {0xD0}}, // shift address
	{0xCB,    15,   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x54,0x54,0x54,0x54,0x00,0x54}},

//CBEx
	{0x00,    1,    {0xE0}}, // shift address
	{0xCB,    9,    {0x00,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},		


//---------GOA timing------------------
//CC8x
	{0x00,    1,    {0x80}}, // shift address
	{0xCC,    10,   {0x00,0x00,0x00,0x00,0x0c,0x0a,0x10,0x0e,0x00,0x02}},
//CC9x
	{0x00,    1,    {0x90}}, // shift address
	{0xCC,    15,   {0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0b}},
//CCAx
	{0x00,    1,    {0xA0}}, // shift address
	{0xCC,    15,   {0x09,0x0f,0x0d,0x00,0x01,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
//CCBx
	{0x00,    1,    {0xB0}}, // shift address
	{0xCC,    10,   {0x00,0x00,0x00,0x00,0x0d,0x0f,0x09,0x0b,0x00,0x05}},
//CCCx
	{0x00,    1,    {0xC0}}, // shift address
	{0xCC,    15,   {0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e}},
//CCDx
	{0x00,    1,    {0xD0}}, // shift address
	{0xCC,    15,   {0x10,0x0a,0x0c,0x00,0x06,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},


//GAMMA 2.2+
	{0x00,    1,    {0x00}},
	{0xE1,    16,   {0x00,0x07,0x0F,0x11,0x09,0x0E,0x09,0x05,0x06,0x08,0x0D,0x06,0x1D,0x17,0x0D,0x06}},
// 255  251  247  239  231  203	175  147  108  80   52   24   16    8     4    0								
//GAMMA 2.2-
	{0x00,    1,    {0x00}},
	{0xE2,    16,   {0x00,0x07,0x0F,0x11,0x09,0x0C,0x0E,0x0F,0x00,0x06,0x0F,0x0C,0x13,0x0D,0x09,0x01}},
//255  251  247  239  231  203	175  147  108  80   52   24   16    8     4    0	


 //       {0x00,    1,   {0x00}},
//	{0xE5,    16,  {0x00,0x0B,0x11,0x0F,0x07,0x0D,0x06,0x03,0x09,0x0B,0x0E,0x02,0x10,0x0E,0x07,0x06}},

 //       {0x00,   1,   {0x00}},
//	{0xE6,   16,  {0x00,0x0D,0x11,0x0D,0x06,0x0D,0x0B,0x0B,0x04,0x09,0x0D,0x00,0x07,0x09,0x03,0x01}},

  //      {0x00,   1,   {0x00}}, // shift address
//	{0x26,   1,   {0x00}},

	{0x00,    1,   {0x00}}, 		  // shift address	
	{0xff,    3,   {0x00,0x00,0x00}},// enable EXTC
	{0x00,    1,   {0x80}}, 		  // shift address
	{0xff,    2,   {0x00,0x00}},	  // enable Orise mode    
	
	{0x35,    1,   {0x00}},//TE
	
	{0x11,    1,   {0x00}},
	{REGFLAG_DELAY, 150, {}},

	{0x29,    1,   {0x00}},
	{REGFLAG_DELAY, 60, {}},
 
    {0x35,1,{0x01}},    //TE on, mode 1 20120618
 
   // Note
   // Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
 
   // Setting ending by predefined flag
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static int get_lcd_id(void)
{
	mt_set_gpio_mode(GPIO_LCD_ID_PIN,0);
	mt_set_gpio_dir(GPIO_LCD_ID_PIN,0);
	mt_set_gpio_pull_enable(GPIO_LCD_ID_PIN,1);
	mt_set_gpio_pull_select(GPIO_LCD_ID_PIN,0);
	MDELAY(1);
	
	return mt_get_gpio_in(GPIO_LCD_ID_PIN);
}
 
static struct LCM_setting_table lcm_set_window[] = {
   {0x2A,  4,  {0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
   {0x2B,  4,  {0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};
 
 
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
   {0x11, 1, {0x00}},
   {REGFLAG_DELAY, 200, {}},
 
    // Display ON
   {0x29, 1, {0x00}},
   {REGFLAG_DELAY, 5, {}},
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};
 
 
static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
   // Sleep Mode On
   {0x10, 1, {0x00}},
   {REGFLAG_DELAY, 200, {}},
   // Display off sequence
   {0x28, 1, {0x00}},
 
 
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};
 
 
static struct LCM_setting_table lcm_backlight_level_setting[] = {
   {0x51, 1, {0xFF}},
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};
 
 
static struct LCM_setting_table lcm_compare_id_setting[] = {
   {0x00,  1,  {0x50}},
   {REGFLAG_END_OF_TABLE, 0x00, {}}
};
 
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
               
            case REGFLAG_END_OF_TABLE :
                break;
               
            default:
               dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
           }
    }
   
}
 
 
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}
 
 
static void lcm_get_params(LCM_PARAMS *params)
{
       memset(params, 0, sizeof(LCM_PARAMS));
   
       params->type   = LCM_TYPE_DSI;
 
       params->width  = FRAME_WIDTH;
       params->height = FRAME_HEIGHT;
 
       // enable tearing-free
       params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
       params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;
 
        #if (LCM_DSI_CMD_MODE)
          params->dsi.mode   = CMD_MODE;
        #else
          params->dsi.mode   = SYNC_PULSE_VDO_MODE;
        #endif
   
       // DSI
       /* Command mode setting */
       params->dsi.LANE_NUM                = LCM_TWO_LANE;
       //The following defined the fomat for data coming from LCD engine.
       params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
       params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
       params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
       params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
 
       // Highly depends on LCD driver capability.
       // Not support in MT6573
       params->dsi.packet_size=256;
 
       // Video mode setting       
       params->dsi.intermediat_buffer_num = 2;
 
       params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
 
      
        params->dsi.vertical_sync_active = 4;//--4
        params->dsi.vertical_backporch = 8;//--6
        params->dsi.vertical_frontporch = 8;//--6
        params->dsi.vertical_active_line = FRAME_HEIGHT; 
    /*
        params->dsi.horizontal_sync_active = 6;//---6
        params->dsi.horizontal_backporch =70;//--30
        params->dsi.horizontal_frontporch =70;//--30
    */  
      // 20120618 modify. for ID 01,8b,80,09
        params->dsi.horizontal_sync_active = 6;//---6
        params->dsi.horizontal_backporch =37;//--30
        params->dsi.horizontal_frontporch =37;//--30
    
        /*  // 20120618 modify. for ID 02,8b,80,09
        params->dsi.horizontal_sync_active = 6;//---6
        params->dsi.horizontal_backporch =23;//--30
        params->dsi.horizontal_frontporch =24;//--30
         */
        params->dsi.horizontal_active_pixel = FRAME_WIDTH;
 
            // Bit rate calculation
    params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=0;		// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =18;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
 
      /* ESD or noise interference recovery For video mode LCM only. */ // Send TE packet to LCM in a period of n frames and check the response.
           params->dsi.lcm_int_te_monitor = 0;// FALSE;
           params->dsi.lcm_int_te_period = 1; // Unit : frames
 
           // Need longer FP for more opportunity to do int. TE monitor applicably.
           if(params->dsi.lcm_int_te_monitor)
           params->dsi.vertical_frontporch *= 2;
 
           // Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.)
           #ifdef BIRD_ESD_CHECK
          
           params->dsi.lcm_ext_te_monitor = 1;// FALSE;
           #else
           params->dsi.lcm_ext_te_monitor = 0;
           #endif
          // Non-continuous clock
          params->dsi.noncont_clock = 1;// TRUE;
         params->dsi.noncont_clock_period = 2; // Unit : frames
}
 
 
static unsigned int lcm_compare_id(void)
{
 
    int   array[4];
   char  buffer[5];
   char  id_high=0;
   char  id_low=0;
   int   id=0;

#ifdef BUILD_LK
		  pmic_config_interface(0x0532,5,0x7,5);//add by libo for VGP2 POWER ON
		  pmic_config_interface(0x050C,1,0x1,15);
#else
		  hwPowerOn(MT6323_POWER_LDO_VGP2,VOL_2800,"LCM");
#endif
    MDELAY(100);

   
   SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
   SET_RESET_PIN(0);
   MDELAY(10);
   SET_RESET_PIN(1);
   MDELAY(10);
 
   array[0] = 0x00053700;// read id return two byte,version and id
   dsi_set_cmdq(array, 1, 1);
   read_reg_v2(0xA1,buffer, 5);
 
   
   id_high = buffer[2];            //should be 0x80
   id_low  = buffer[3];            //should be 0x18
   id      = (id_high<<8)|id_low;  //should be 0x8018
 
   
 

	 Lcd_Log("%s, id = 0x%08x\n", __func__, id); 
 
   return (LCM_ID == id&&get_lcd_id()==1)?1:0;
}
 
 
static void lcm_init(void)
{
	  int flag=0;
	  
#ifdef BUILD_LK
		  pmic_config_interface(0x0532,5,0x7,5);//add by libo for VGP2 POWER ON
		  pmic_config_interface(0x050C,1,0x1,15);
#else
		  hwPowerOn(MT6323_POWER_LDO_VGP2,VOL_2800,"LCM");
#endif
		  MDELAY(100);

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);
  
   Lcd_Log("mycat otm8018b flag=%d  \n",flag);

	 push_table(lcm_initialization_setting1, sizeof(lcm_initialization_setting1) / sizeof(struct LCM_setting_table), 1);
}
 
 
static void lcm_suspend(void)
{
   push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
   SET_RESET_PIN(0);
   MDELAY(1);
   SET_RESET_PIN(1);
  
}
 
 
static void lcm_resume(void)
{

   lcm_init();
 
   push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

}
 
 
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
   unsigned int x0 = x;
   unsigned int y0 = y;
   unsigned int x1 = x0 + width - 1;
   unsigned int y1 = y0 + height - 1;
 
   unsigned char x0_MSB = ((x0>>8)&0xFF);
   unsigned char x0_LSB = (x0&0xFF);
   unsigned char x1_MSB = ((x1>>8)&0xFF);
   unsigned char x1_LSB = (x1&0xFF);
   unsigned char y0_MSB = ((y0>>8)&0xFF);
   unsigned char y0_LSB = (y0&0xFF);
   unsigned char y1_MSB = ((y1>>8)&0xFF);
   unsigned char y1_LSB = (y1&0xFF);
 
   unsigned int data_array[16];
 
   data_array[0]= 0x00053902;
   data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
   data_array[2]= (x1_LSB);
   data_array[3]= 0x00053902;
   data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
   data_array[5]= (y1_LSB);
   data_array[6]= 0x002c3909;
 
   dsi_set_cmdq(&data_array, 7, 0);
 
}
 
static void lcm_setbacklight(unsigned int level)
{
   unsigned int default_level = 145;
   unsigned int mapped_level = 0;
 
   //for LGE backlight IC mapping table
   if(level > 255) 
           level = 255;
 
   if(level >0) 
           mapped_level = default_level+(level)*(255-default_level)/(255);
   else
           mapped_level=0;
 
   // Refresh value of backlight level.
   lcm_backlight_level_setting[0].para_list[0] = mapped_level;
 
   push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
 
static unsigned int lcm_esd_recover()
{
	
	 lcm_init();
   
   push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}
LCM_DRIVER otm8018b_truly_tft3P4608_dsi_fwvga_lcm_drv = 
{
    .name			= "otm8018b_truly_tft3P4608_dsi_fwvga",
	.set_util_funcs = lcm_set_util_funcs,
    .compare_id    = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)

    .update         = lcm_update,
#endif
#if defined (BIRD_ESD_CHECK)

  .esd_recover = lcm_esd_recover,
#endif
    };
