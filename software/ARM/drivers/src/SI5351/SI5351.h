/*
 * SI5351.h
 *
 *  Created on: 28 Jun 2013
 *      Author: dario
 */

#ifndef SI5351_H_
#define SI5351_H_

// --------------------------------------------------------
//					REG 0 - Device Status
// --------------------------------------------------------
#define DEV_STS_REG				0

#define SYS_INIT_MSK			0x80
#define SYS_INIT_SH				7
#define LOL_B_MSK				0x40
#define LOL_B_SH				6
#define LOL_A_MSK				0x20
#define LOL_A_SH				5
#define LOS_MSK					0x10
#define LOS_SH					4
#define REVID_MSK				0x03
#define REVID_SH				0

// --------------------------------------------------------
//					REG 1 - Interrupt Status Sticky
// --------------------------------------------------------
#define INT_STS_STKY_REG		1

#define SYS_INIT_STKY_MSK		0x80
#define SYS_INIT_STKY_SH		7
#define LOL_B_STKY_MSK			0x40
#define LOL_B_STKY_SH			6
#define LOL_A_STKY_MSK			0x20
#define LOL_A_STKY_SH			5
#define LOS_STKY_MSK			0x10
#define LOS_STKY_SH				4

// --------------------------------------------------------
//					REG 2 - Interrupt Status Mask
// --------------------------------------------------------
#define INT_STS_MASK_REG		2

#define SYS_INIT_MASK_MSK		0x80
#define SYS_INIT_MASK_SH		7
#define LOL_B_MASK_MSK			0x40
#define LOL_B_MASK_SH			6
#define LOL_A_MASK_MSK			0x20
#define LOL_A_MASK_SH			5
#define LOS_MASK_MSK			0x10
#define LOS_MASK_SH				4

// --------------------------------------------------------
//					REG 3 - Output Enable Control
// --------------------------------------------------------
#define OUT_EN_CTRL_REG			3

#define CLK7_OEB_MSK			0x80
#define CLK7_OEB_SH				7
#define CLK6_OEB_MSK			0x40
#define CLK6_OEB_SH				6
#define CLK5_OEB_MSK			0x20
#define CLK5_OEB_SH				5
#define CLK4_OEB_MSK			0x10
#define CLK4_OEB_SH				4
#define CLK3_OEB_MSK			0x08
#define CLK3_OEB_SH				3
#define CLK2_OEB_MSK			0x04
#define CLK2_OEB_SH				2
#define CLK1_OEB_MSK			0x02
#define CLK1_OEB_SH				1
#define CLK0_OEB_MSK			0x01
#define CLK0_OEB_SH				0

// --------------------------------------------------------
//					REG 9 - OEB Pin Enable Control
// --------------------------------------------------------
#define OEB_PIN_EN_CTRL_REG		9

#define OEB_CLK7_MSK			0x80
#define OEB_CLK7_SH				7
#define OEB_CLK6_MSK			0x40
#define OEB_CLK6_SH				6
#define OEB_CLK5_MSK			0x20
#define OEB_CLK5_SH				5
#define OEB_CLK4_MSK			0x10
#define OEB_CLK4_SH				4
#define OEB_CLK3_MSK			0x08
#define OEB_CLK3_SH				3
#define OEB_CLK2_MSK			0x04
#define OEB_CLK2_SH				2
#define OEB_CLK1_MSK			0x02
#define OEB_CLK1_SH				1
#define OEB_CLK0_MSK			0x01
#define OEB_CLK0_SH				0

// --------------------------------------------------------
//					REG 15 - PLL Input Source
// --------------------------------------------------------
#define PLL_IN_SRC_REG			15

#define PLLB_SRC_MSK			0x08
#define PLLB_SRC_SH				3
#define PLLA_SRC_MSK			0x04
#define PLLA_SRC_SH				2

// --------------------------------------------------------
//					REG 16 - CLK0 Control
// --------------------------------------------------------
#define CLK0_CTRL_REG			16

#define CLK0_PDN_MSK			0x80
#define CLK0_PDN_SH				7
#define MS0_INT_MSK				0x40
#define MS0_INT_SH				6
#define MS0_SRC_MSK				0x20
#define MS0_SRC_SH				5
#define CLK0_INV_MSK			0x10
#define CLK0_INV_SH				4
#define CLK0_SRC_MSK			0x0C
#define CLK0_SRC_SH				2
#define CLK0_IDRV_MSK			0x03
#define CLK0_IDRV_SH			0

// --------------------------------------------------------
//					REG 17 - CLK1 Control
// --------------------------------------------------------
#define CLK1_CTRL_REG			17

#define CLK1_PDN_MSK			0x80
#define CLK1_PDN_SH				7
#define MS1_INT_MSK				0x40
#define MS1_INT_SH				6
#define MS1_SRC_MSK				0x20
#define MS1_SRC_SH				5
#define CLK1_INV_MSK			0x10
#define CLK1_INV_SH				4
#define CLK1_SRC_MSK			0x0C
#define CLK1_SRC_SH				2
#define CLK1_IDRV_MSK			0x03
#define CLK1_IDRV_SH			0

// --------------------------------------------------------
//					REG 18 - CLK2 Control
// --------------------------------------------------------
#define CLK2_CTRL_REG			18

#define CLK2_PDN_MSK			0x80
#define CLK2_PDN_SH				7
#define MS2_INT_MSK				0x40
#define MS2_INT_SH				6
#define MS2_SRC_MSK				0x20
#define MS2_SRC_SH				5
#define CLK2_INV_MSK			0x10
#define CLK2_INV_SH				4
#define CLK2_SRC_MSK			0x0C
#define CLK2_SRC_SH				2
#define CLK2_IDRV_MSK			0x03
#define CLK2_IDRV_SH			0

// --------------------------------------------------------
//					REG 19 - CLK3 Control
// --------------------------------------------------------
#define CLK3_CTRL_REG			19

#define CLK3_PDN_MSK			0x80
#define CLK3_PDN_SH				7
#define MS3_INT_MSK				0x40
#define MS3_INT_SH				6
#define MS3_SRC_MSK				0x20
#define MS3_SRC_SH				5
#define CLK3_INV_MSK			0x10
#define CLK3_INV_SH				4
#define CLK3_SRC_MSK			0x0C
#define CLK3_SRC_SH				2
#define CLK3_IDRV_MSK			0x03
#define CLK3_IDRV_SH			0

// --------------------------------------------------------
//					REG 20 - CLK4 Control
// --------------------------------------------------------
#define CLK4_CTRL_REG			20

#define CLK4_PDN_MSK			0x80
#define CLK4_PDN_SH				7
#define MS4_INT_MSK				0x40
#define MS4_INT_SH				6
#define MS4_SRC_MSK				0x20
#define MS4_SRC_SH				5
#define CLK4_INV_MSK			0x10
#define CLK4_INV_SH				4
#define CLK4_SRC_MSK			0x0C
#define CLK4_SRC_SH				2
#define CLK4_IDRV_MSK			0x03
#define CLK4_IDRV_SH			0

// --------------------------------------------------------
//					REG 21 - CLK5 Control
// --------------------------------------------------------
#define CLK5_CTRL_REG			21

#define CLK5_PDN_MSK			0x80
#define CLK5_PDN_SH				7
#define MS5_INT_MSK				0x40
#define MS5_INT_SH				6
#define MS5_SRC_MSK				0x20
#define MS5_SRC_SH				5
#define CLK5_INV_MSK			0x10
#define CLK5_INV_SH				4
#define CLK5_SRC_MSK			0x0C
#define CLK5_SRC_SH				2
#define CLK5_IDRV_MSK			0x03
#define CLK5_IDRV_SH			0

// --------------------------------------------------------
//					REG 22 - CLK6 Control
// --------------------------------------------------------
#define CLK6_CTRL_REG			22

#define CLK6_PDN_MSK			0x80
#define CLK6_PDN_SH				7
#define FBA_INT_MSK				0x40
#define FBA_INT_SH				6
#define MS6_SRC_MSK				0x20
#define MS6_SRC_SH				5
#define CLK6_INV_MSK			0x10
#define CLK6_INV_SH				4
#define CLK6_SRC_MSK			0x0C
#define CLK6_SRC_SH				2
#define CLK6_IDRV_MSK			0x03
#define CLK6_IDRV_SH			0

// --------------------------------------------------------
//					REG 23 - CLK7 Control
// --------------------------------------------------------
#define CLK7_CTRL_REG			23

#define CLK7_PDN_MSK			0x80
#define CLK7_PDN_SH				7
#define FBB_INT_MSK				0x40
#define FBB_INT_SH				6
#define MS7_SRC_MSK				0x20
#define MS7_SRC_SH				5
#define CLK7_INV_MSK			0x10
#define CLK7_INV_SH				4
#define CLK7_SRC_MSK			0x0C
#define CLK7_SRC_SH				2
#define CLK7_IDRV_MSK			0x03
#define CLK7_IDRV_SH			0

// --------------------------------------------------------
//					REG 24 - CLK3-0 Disable State
// --------------------------------------------------------
#define CLK3_0_DIS_STATE_REG	24

#define CLK3_DIS_STATE_MSK		0xC0
#define CLK3_DIS_STATE_SH		6
#define CLK2_DIS_STATE_MSK		0x30
#define CLK2_DIS_STATE_SH		4
#define CLK1_DIS_STATE_MSK		0x0C
#define CLK1_DIS_STATE_SH		2
#define CLK0_DIS_STATE_MSK		0x03
#define CLK0_DIS_STATE_SH		0

// --------------------------------------------------------
//					REG 25 - CLK7-4 Disable State
// --------------------------------------------------------
#define CLK7_4_DIS_STATE_REG	25

#define CLK7_DIS_STATE_MSK		0xC0
#define CLK7_DIS_STATE_SH		6
#define CLK6_DIS_STATE_MSK		0x30
#define CLK6_DIS_STATE_SH		4
#define CLK5_DIS_STATE_MSK		0x0C
#define CLK5_DIS_STATE_SH		2
#define CLK4_DIS_STATE_MSK		0x03
#define CLK4_DIS_STATE_SH		0

// --------------------------------------------------------
//					REG 42 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_42_REG			42

#define MS0_P3_15_8_MSK			0xFF
#define MS0_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 43 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_43_REG			43

#define MS0_P3_7_0_MSK			0xFF
#define MS0_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 44 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_44_REG			44

#define R0_DIV_2_0_MSK			0x70
#define R0_DIV_2_0_SH			4
#define MS0_P1_17_16_MSK		0x03
#define MS0_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 45 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_45_REG			45

#define MS0_P1_15_8_MSK			0xFF
#define MS0_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 46 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_46_REG			46

#define MS0_P1_7_0_MSK			0xFF
#define MS0_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 47 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_47_REG			47

#define MS0_P3_19_16_MSK		0xF0
#define MS0_P3_19_16_SH			4
#define MS0_P2_19_16_MSK		0x0F
#define MS0_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 48 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_48_REG			48

#define MS0_P2_15_8_MSK			0xFF
#define MS0_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 49 - Multisynth0 Parameters
// --------------------------------------------------------
#define MS0_PAR_49_REG			49

#define MS0_P2_7_0_MSK			0xFF
#define MS0_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 50 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_50_REG			50

#define MS1_P3_15_8_MSK			0xFF
#define MS1_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 51 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_51_REG			51

#define MS1_P3_7_0_MSK			0xFF
#define MS1_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 52 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_52_REG			52

#define R1_DIV_2_0_MSK			0x70
#define R1_DIV_2_0_SH			4
#define MS1_P1_17_16_MSK		0x03
#define MS1_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 53 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_53_REG			53

#define MS1_P1_15_8_MSK			0xFF
#define MS1_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 54 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_54_REG			54

#define MS1_P1_7_0_MSK			0xFF
#define MS1_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 55 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_55_REG			55

#define MS1_P3_19_16_MSK		0xF0
#define MS1_P3_19_16_SH			4
#define MS1_P2_19_16_MSK		0x0F
#define MS1_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 56 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_56_REG			56

#define MS1_P2_15_8_MSK			0xFF
#define MS1_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 57 - Multisynth1 Parameters
// --------------------------------------------------------
#define MS1_PAR_57_REG			57

#define MS1_P2_7_0_MSK			0xFF
#define MS1_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 58 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_58_REG			58

#define MS2_P3_15_8_MSK			0xFF
#define MS2_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 59 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_59_REG			59

#define MS2_P3_7_0_MSK			0xFF
#define MS2_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 60 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_60_REG			60

#define R2_DIV_2_0_MSK			0x70
#define R2_DIV_2_0_SH			4
#define MS2_P1_17_16_MSK		0x03
#define MS2_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 61 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_61_REG			61

#define MS2_P1_15_8_MSK			0xFF
#define MS2_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 62 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_62_REG			62

#define MS2_P1_7_0_MSK			0xFF
#define MS2_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 63 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_63_REG			63

#define MS2_P3_19_16_MSK		0xF0
#define MS2_P3_19_16_SH			4
#define MS2_P2_19_16_MSK		0x0F
#define MS2_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 64 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_64_REG			64

#define MS2_P2_15_8_MSK			0xFF
#define MS2_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 65 - Multisynth2 Parameters
// --------------------------------------------------------
#define MS2_PAR_65_REG			65

#define MS2_P2_7_0_MSK			0xFF
#define MS2_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 66 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_66_REG			66

#define MS3_P3_15_8_MSK			0xFF
#define MS3_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 67 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_67_REG			67

#define MS3_P3_7_0_MSK			0xFF
#define MS3_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 68 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_68_REG			68

#define R3_DIV_2_0_MSK			0x70
#define R3_DIV_2_0_SH			4
#define MS3_P1_17_16_MSK		0x03
#define MS3_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 69 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_69_REG			69

#define MS3_P1_15_8_MSK			0xFF
#define MS3_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 70 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_70_REG			70

#define MS3_P1_7_0_MSK			0xFF
#define MS3_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 71 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_71_REG			71

#define MS3_P3_19_16_MSK		0xF0
#define MS3_P3_19_16_SH			4
#define MS3_P2_19_16_MSK		0x0F
#define MS3_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 72 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_72_REG			72

#define MS3_P2_15_8_MSK			0xFF
#define MS3_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 73 - Multisynth3 Parameters
// --------------------------------------------------------
#define MS3_PAR_73_REG			73

#define MS3_P2_7_0_MSK			0xFF
#define MS3_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 74 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_74_REG			74

#define MS4_P3_15_8_MSK			0xFF
#define MS4_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 75 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_75_REG			75

#define MS4_P3_7_0_MSK			0xFF
#define MS4_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 76 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_76_REG			76

#define R4_DIV_2_0_MSK			0x70
#define R4_DIV_2_0_SH			4
#define MS4_P1_17_16_MSK		0x03
#define MS4_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 77 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_77_REG			77

#define MS4_P1_15_8_MSK			0xFF
#define MS4_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 78 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_78_REG			78

#define MS4_P1_7_0_MSK			0xFF
#define MS4_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 79 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_79_REG			79

#define MS4_P3_19_16_MSK		0xF0
#define MS4_P3_19_16_SH			4
#define MS4_P2_19_16_MSK		0x0F
#define MS4_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 80 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_80_REG			80

#define MS4_P2_15_8_MSK			0xFF
#define MS4_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 81 - Multisynth4 Parameters
// --------------------------------------------------------
#define MS4_PAR_81_REG			81

#define MS4_P2_7_0_MSK			0xFF
#define MS4_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 82 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_82_REG			82

#define MS5_P3_15_8_MSK			0xFF
#define MS5_P3_15_8_SH			0

// --------------------------------------------------------
//					REG 83 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_83_REG			83

#define MS5_P3_7_0_MSK			0xFF
#define MS5_P3_7_0_SH			0

// --------------------------------------------------------
//					REG 84 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_84_REG			84

#define R5_DIV_2_0_MSK			0x70
#define R5_DIV_2_0_SH			4
#define MS5_P1_17_16_MSK		0x03
#define MS5_P1_17_16_SH			0

// --------------------------------------------------------
//					REG 85 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_85_REG			85

#define MS5_P1_15_8_MSK			0xFF
#define MS5_P1_15_8_SH			0

// --------------------------------------------------------
//					REG 86 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_86_REG			86

#define MS5_P1_7_0_MSK			0xFF
#define MS5_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 87 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_87_REG			87

#define MS5_P3_19_16_MSK		0xF0
#define MS5_P3_19_16_SH			4
#define MS5_P2_19_16_MSK		0x0F
#define MS5_P2_19_16_SH			0

// --------------------------------------------------------
//					REG 88 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_88_REG			88

#define MS5_P2_15_8_MSK			0xFF
#define MS5_P2_15_8_SH			0

// --------------------------------------------------------
//					REG 89 - Multisynth5 Parameters
// --------------------------------------------------------
#define MS5_PAR_89_REG			89

#define MS5_P2_7_0_MSK			0xFF
#define MS5_P2_7_0_SH			0

// --------------------------------------------------------
//					REG 90 - Multisynth6 Parameters
// --------------------------------------------------------
#define MS6_PAR_90_REG			90

#define MS6_P1_7_0_MSK			0xFF
#define MS6_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 91 - Multisynth7 Parameters
// --------------------------------------------------------
#define MS7_PAR_91_REG			91

#define MS7_P1_7_0_MSK			0xFF
#define MS7_P1_7_0_SH			0

// --------------------------------------------------------
//					REG 92 - CLK_6_7 Output Divider
// --------------------------------------------------------
#define CLK_6_7_OUTPUT_DIV_REG	92

#define R7_DIV_2_0_MSK			0x70
#define R7_DIV_2_0_SH			4
#define R6_DIV_2_0_MSK			0x07
#define R6_DIV_2_0_SH			0

// --------------------------------------------------------
//					REG 165 - CLK_0 Initial Phase Offset
// --------------------------------------------------------
#define CLK0_PHASE_OFFS_REG		165

#define CLK0_PHOFF_6_0_MSK		0x7F
#define CLK0_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 166 - CLK_1 Initial Phase Offset
// --------------------------------------------------------
#define CLK1_PHASE_OFFS_REG		166

#define CLK1_PHOFF_6_0_MSK		0x7F
#define CLK1_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 167 - CLK_2 Initial Phase Offset
// --------------------------------------------------------
#define CLK2_PHASE_OFFS_REG		167

#define CLK2_PHOFF_6_0_MSK		0x7F
#define CLK2_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 168 - CLK_3 Initial Phase Offset
// --------------------------------------------------------
#define CLK3_PHASE_OFFS_REG		168

#define CLK3_PHOFF_6_0_MSK		0x7F
#define CLK3_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 169 - CLK_4 Initial Phase Offset
// --------------------------------------------------------
#define CLK4_PHASE_OFFS_REG		169

#define CLK4_PHOFF_6_0_MSK		0x7F
#define CLK4_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 170 - CLK_5 Initial Phase Offset
// --------------------------------------------------------
#define CLK5_PHASE_OFFS_REG		170

#define CLK5_PHOFF_6_0_MSK		0x7F
#define CLK5_PHOFF_6_0_SH		0

// --------------------------------------------------------
//					REG 177 - PLL Reset
// --------------------------------------------------------
#define PLL_RESET_REG			177

#define PLLB_RST_MSK			0x80
#define PLLB_RST_SH				7
#define PLLA_RST_MSK			0x20
#define PLLA_RST_SH				5

// --------------------------------------------------------
//					REG 183 - Crystal Internal Load Cap
// --------------------------------------------------------
#define XTAL_CL_REG				183

#define XTAL_CL_MSK				0xC0
#define XTAL_CL_SH				6

void Si5351_Initialize(void);
void Si5351_OutputEnable(uint8_t bEnable);
void Si5351_PLLReset(void);
void Si5351_OutputFrequency(uint32_t dwFreqHz);


#endif /* SI5351_H_ */
