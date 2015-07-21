/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/



#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
    #include <linux/string.h>
   
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)

#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0x100  // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define LCM_ID       (0x55)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
     
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},


    // Display ON
    {0x29, 1, {0x00}},
  ///  {REGFLAG_DELAY, 120, {}},
	 {0x32, 1, {0x00}},
{REGFLAG_DELAY, 20, {}},
  {REGFLAG_END_OF_TABLE, 0x00, {}}


};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_initialization_setting[] = {
{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xB1,3,{0x7C,0x00,0x00}},
{0xB6,1,{0x03}},
{0xB7,2,{0x72,0x72}},
{0xB8,4,{0x01,0x06,0x05,0x04}},
{0xBB,1,{0x55}},
{0xBC,3,{0x02,0x00,0x00}},    
{0xBD,5,{0x01,0x4E,0x10,0x20,0x01}},
{0xC9,6,{0x61,0x06,0x0D,0x17,0x17,0x00}},
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
{0xB0,3,{0x0C,0x0C,0x0C}},
{0xB1,3,{0x0C,0x0C,0x0C}},
{0xB2,3,{0x02,0x02,0x02}},
{0xB3,3,{0x10,0x10,0x10}},
{0xB4,3,{0x06,0x06,0x06}},
{0xB6,3,{0x54,0x54,0x54}},
{0xB7,3,{0x24,0x24,0x24}},
{0xB8,3,{0x34,0x34,0x34}},
{0xB9,3,{0x34,0x34,0x34}},
{0xBA,3,{0x14,0x14,0x14}},
{0xBC,3,{0x00,0x98,0x00}},
{0xBD,3,{0x00,0x98,0x00}},
{0xC1,1,{0x01}},
{0xBf,1,{0x46}},
{0xC2,1,{0x00}},
{0xD0,4,{0x0F,0x0F,0x10,0x10}},
{0xD1,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xD2,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xD3,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xD4,4,{0x03,0xF6,0x03,0xFB}},
{0xD5,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xD6,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xD7,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xD8,4,{0x03,0xF6,0x03,0xFB}},
{0xD9,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xDD,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xDE,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xDF,4,{0x03,0xF6,0x03,0xFB}},
{0xE0,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xE1,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xE2,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xE3,4,{0x03,0xF6,0x03,0xFB}},
{0xE4,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xE5,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xE6,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xE7,4,{0x03,0xF6,0x03,0xFB}},
{0xE8,16,{0x00,0x00,0x00,0x62,0x00,0x90,0x00,0xAE,0x00,0xC5,0x00,0xEA,0x01,0x07,0x01,0x34}},
{0xE9,16,{0x01,0x58,0x01,0x8F,0x01,0xBB,0x01,0xFF,0x02,0x37,0x02,0x39,0x02,0x6E,0x02,0xA9}},
{0xEA,16,{0x02,0xD0,0x03,0x07,0x03,0x2C,0x03,0x5F,0x03,0x81,0x03,0xAC,0x03,0xC3,0x03,0xDD}},
{0xEB,4,{0x03,0xF6,0x03,0xFB}},
{0x35,1,{0x00}},
{0x3A,1,{0x77}},
{0x36,1,{0x00}},
{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 200, {}},
    
    // Display ON
    {0x29, 1, {0x00}},
 {REGFLAG_DELAY, 20, {}},

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
    params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_DISABLED;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
    params->dsi.mode   =   SYNC_EVENT_VDO_MODE;  //SYNC_PULSE_VDO_MODE;
    
    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_TWO_LANE;

    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;



    //params->dsi.packet_size=256;
    params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
    
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.word_count=540*3;	//DSI CMD mode need set these two bellow params, different to 6577    480*3
    
    params->dsi.vertical_active_line=960;///854
  
     params->dsi.vertical_sync_active				       = 2;//6;2
    params->dsi.vertical_backporch					=60;//10;// 70;20
    params->dsi.vertical_frontporch					= 60;//10;//70;	20
    params->dsi.vertical_active_line				= FRAME_HEIGHT;
    
    params->dsi.horizontal_sync_active			= 2;//6;2
    params->dsi.horizontal_backporch				= 60;//80;//22;80
    params->dsi.horizontal_frontporch				= 60;//80;//120;80
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

    params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=0;//0;		// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =19;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	//14

}


static void lcm_init(void)
{
#if defined(BUILD_LK)
	upmu_set_rg_vgp2_vosel(5);
	upmu_set_rg_vgp2_en(1);

	upmu_set_rg_vgp3_vosel(3);
	upmu_set_rg_vgp3_en(1);	
#else
	hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "Lance_LCM");
       hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "Lance_LCM");
#endif

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);
push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
    unsigned int data_array[16];
#if defined(BUILD_LK)

#else
      data_array[0] = 0x00002200;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
#endif
    data_array[0] = 0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);

    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);

}


static void lcm_resume(void)
{
	lcm_init();
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
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);
    
    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);
}

static unsigned int lcm_compare_id(void)
{

    		return 1;
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35517_liken_HT8459RKF_dsi_qhd_lcm_drv = 
{
    .name			= "nt35517_liken_HT8459RKF_dsi_qhd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
};
