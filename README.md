# csv2bf
csv converts the bit field structure to facilitate register reading and writing.

# csv2bf
把写有寄存器列表的表格，转成位域结构体。

表格大概长这样。

| IP_Name | Block   | Address     | Bits | Register Name                  | R/W  | Width |
| ------- | ------- | ----------- | ---- | ------------------------------ | ---- | ----- |
| V_NR    | V_NR_M0 | 0x0500_0000 | [31] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [30] | rstn_sw_cclk_wb_next           | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [29] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [28] | rstn_sw_cclk_agent_tnr_w_c     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [27] | rstn_sw_cclk_agent_tnr_w_y     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [26] | rstn_sw_cclk_sc                | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [25] | rstn_sw_cclk_rback             | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [24] | rstn_sw_cclk_crop              | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [23] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [22] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [21] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [20] | rstn_sw_cclk_tnr               | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [19] | rstn_sw_cclk_agent_tnr_r_c     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [18] | rstn_sw_cclk_agent_tnr_r_y     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [17] | rstn_sw_cclk_overlay           | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [16] | rstn_sw_cclk_next_r            | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [15] | rstn_sw_pclk_bl_pre_timing_gen | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [14] | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [13] | rstn_sw_pclk_agent_tnr_w_c     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [12] | rstn_sw_pclk_rback             | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [11] | rstn_sw_pclk_agent_tnr_w_y     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [10] | rstn_sw_pclk_sc                | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [9]  | rstn_sw_pclk_fstart            | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [8]  | rstn_sw_pclk_crop              | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [7]  | rstn_sw_pclk_hbnr_bmnr         | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [6]  | rstn_sw_pclk_afbd_fifo         | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [5]  | spare                          | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [4]  | rstn_sw_pclk_tnr               | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [3]  | rstn_sw_pclk_agent_tnr_r_c     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [2]  | rstn_sw_pclk_agent_tnr_r_y     | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [1]  | rstn_sw_pclk_overlay           | R/W  | 1     |
| V_NR    | V_NR_M0 | 0x0500_0000 | [0]  | rstn_sw_pclk_next_r            | R/W  | 1     |

生成的位域结构体是这样

``` c
//
// V_NR_M0_0000
//
#define _TV303_V_NR_M0_0000 (_TV303_V_NR_BASE + 0x0000U)

#pragma pack(push, 4)
typedef struct
{
    __IO uint32_t rstn_sw_pclk_next_r : 1;             // [0]
    __IO uint32_t rstn_sw_pclk_overlay : 1;            // [1]
    __IO uint32_t rstn_sw_pclk_agent_tnr_r_y : 1;      // [2]
    __IO uint32_t rstn_sw_pclk_agent_tnr_r_c : 1;      // [3]
    __IO uint32_t rstn_sw_pclk_tnr : 1;                // [4]
    __IO uint32_t spare : 1;                           // [5]
    __IO uint32_t rstn_sw_pclk_afbd_fifo : 1;          // [6]
    __IO uint32_t rstn_sw_pclk_hbnr_bmnr : 1;          // [7]
    __IO uint32_t rstn_sw_pclk_crop : 1;               // [8]
    __IO uint32_t rstn_sw_pclk_fstart : 1;             // [9]
    __IO uint32_t rstn_sw_pclk_sc : 1;                 // [10]
    __IO uint32_t rstn_sw_pclk_agent_tnr_w_y : 1;      // [11]
    __IO uint32_t rstn_sw_pclk_rback : 1;              // [12]
    __IO uint32_t rstn_sw_pclk_agent_tnr_w_c : 1;      // [13]
    __IO uint32_t spare_14 : 1;                        // [14]
    __IO uint32_t rstn_sw_pclk_bl_pre_timing_gen : 1;  // [15]
    __IO uint32_t rstn_sw_cclk_next_r : 1;             // [16]
    __IO uint32_t rstn_sw_cclk_overlay : 1;            // [17]
    __IO uint32_t rstn_sw_cclk_agent_tnr_r_y : 1;      // [18]
    __IO uint32_t rstn_sw_cclk_agent_tnr_r_c : 1;      // [19]
    __IO uint32_t rstn_sw_cclk_tnr : 1;                // [20]
    __IO uint32_t spare_21 : 1;                        // [21]
    __IO uint32_t spare_22 : 1;                        // [22]
    __IO uint32_t spare_23 : 1;                        // [23]
    __IO uint32_t rstn_sw_cclk_crop : 1;               // [24]
    __IO uint32_t rstn_sw_cclk_rback : 1;              // [25]
    __IO uint32_t rstn_sw_cclk_sc : 1;                 // [26]
    __IO uint32_t rstn_sw_cclk_agent_tnr_w_y : 1;      // [27]
    __IO uint32_t rstn_sw_cclk_agent_tnr_w_c : 1;      // [28]
    __IO uint32_t spare_29 : 1;                        // [29]
    __IO uint32_t rstn_sw_cclk_wb_next : 1;            // [30]
    __IO uint32_t spare_31 : 1;                        // [31]
} V_NR_M0_0000_TypeDef;
#pragma pack(pop)

typedef union
{
    uint32_t             all;
    V_NR_M0_0000_TypeDef bit;
} V_NR_M0_0000_REG;

#define V_NR_M0_0000 ((V_NR_M0_0000_REG *)((uint32_t)SVP_REG_MIPS(_TV303_V_NR_M0_0000)))

```



# todo

添加 ini 文件配置 CSV 表格的功能。

把CSV 解析换为 libcsv。

从描述中读取位宽功能。

添加描述注释功能。