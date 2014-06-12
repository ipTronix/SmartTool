/*
 * SI5351.c
 *
 *  Created on: 28 Jun 2013
 *      Author: dario
 */

#include "lpc11Uxx.h"
#include "Si5351.h"
#include "I2C/i2c.h"

#define SI5351_ADDR		0xC0
#define F_VCO	675000000

#define SI5351_WRITE_FIELD(reg, field, val)		Si5351_WriteField(reg, field##_MSK, field##_SH, val)
#define SI5351_READ_FIELD(reg, field)			Si5351_ReadField(reg, field##_MSK, field##_SH)

static const uint8_t m_acRegInit[] =
{
  //0,0x00,
  1,0x00,
  2,0x18,
  //3,0xFF,
  3,0x00,
  //4,0x00,
  //5,0x00,
  //6,0x00,
  //7,0x00,
  //8,0x00,
  9,0xff,
  //10,0x00,
  //11,0x00,
  //12,0x00,
  //13,0x00,
  //14,0x00,
 15,0x00,
 //16,0x8F,
 //17,0x9F,
 16,0x0F,
 17,0x1F,
 18,0x83,
 19,0x80,
 20,0x80,
 21,0x80,
 22,0x80,
 23,0x80,
 24,0x00,
 25,0x00,
 26,0x00,
 27,0x01,
 28,0x00,
 29,0x0B,
 30,0x80,
 31,0x00,
 32,0x00,
 33,0x00,
 34,0x00,
 35,0x00,
 36,0x00,
 37,0x00,
 38,0x00,
 39,0x00,
 40,0x00,
 41,0x00,
 42,0x00,
 43,0x01,
 44,0x00,
 45,0x17,
 46,0x00,
 47,0x00,
 48,0x00,
 49,0x00,
 50,0x00,
 51,0x01,
 52,0x00,
 53,0x17,
 54,0x00,
 55,0x00,
 56,0x00,
 57,0x00,
 58,0x00,
 59,0x00,
 60,0x00,
 61,0x00,
 62,0x00,
 63,0x00,
 64,0x00,
 65,0x00,
 66,0x00,
 67,0x00,
 68,0x00,
 69,0x00,
 70,0x00,
 71,0x00,
 72,0x00,
 73,0x00,
 74,0x00,
 75,0x00,
 76,0x00,
 77,0x00,
 78,0x00,
 79,0x00,
 80,0x00,
 81,0x00,
 82,0x00,
 83,0x00,
 84,0x00,
 85,0x00,
 86,0x00,
 87,0x00,
 88,0x00,
 89,0x00,
 90,0x00,
 91,0x00,
 92,0x00,
 93,0x00,
 94,0x00,
 95,0x00,
 96,0x00,
 97,0x00,
 98,0x00,
 99,0x00,
100,0x00,
101,0x00,
102,0x00,
103,0x00,
104,0x00,
105,0x00,
106,0x00,
107,0x00,
108,0x00,
109,0x00,
110,0x00,
111,0x00,
112,0x00,
113,0x00,
114,0x00,
115,0x00,
116,0x00,
117,0x00,
118,0x00,
119,0x00,
120,0x00,
121,0x00,
122,0x00,
123,0x00,
124,0x00,
125,0x00,
126,0x00,
127,0x00,
128,0x00,
129,0x00,
130,0x00,
131,0x00,
132,0x00,
133,0x00,
134,0x00,
135,0x00,
136,0x00,
137,0x00,
138,0x00,
139,0x00,
140,0x00,
141,0x00,
142,0x00,
143,0x00,
144,0x00,
145,0x00,
146,0x00,
147,0x00,
148,0x00,
149,0x00,
150,0x00,
151,0x00,
152,0x00,
153,0x00,
154,0x00,
155,0x00,
156,0x00,
157,0x00,
158,0x00,
159,0x00,
160,0x00,
161,0x00,
162,0x00,
163,0x00,
164,0x00,
165,0x00,
166,0x00,
167,0x00,
168,0x00,
169,0x00,
170,0x00,
171,0x00,
172,0x00,
//173,0x00,
//174,0x00,
//175,0x00,
//176,0x00,
177,0x00,
//178,0x00,
//179,0x00,
//180,0x00,
//181,0x30,
//182,0x00,
183,0x12,
/*
184,0x60,
185,0x60,
186,0x00,
187,0xC0,
188,0x00,
189,0x00,
190,0x00,
191,0x00,
192,0x00,
193,0x00,
194,0x00,
195,0x00,
196,0x00,
197,0x00,
198,0x00,
199,0x00,
200,0x00,
201,0x00,
202,0x00,
203,0x00,
204,0x00,
205,0x00,
206,0x00,
207,0x00,
208,0x00,
209,0x00,
210,0x00,
211,0x00,
212,0x00,
213,0x00,
214,0x00,
215,0x00,
216,0x00,
217,0x00,
218,0x00,
219,0x00,
220,0x00,
221,0x0D,
222,0x00,
223,0x00,
224,0x00,
225,0x00,
226,0x00,
227,0x00,
228,0x00,
229,0x00,
230,0x00,
231,0x00,
232,0x00,
*/
};

static uint8_t Si5351_ReadReg(uint8_t cRegAddr)
{
	uint8_t cRegVal;

	I2CRead(SI5351_ADDR,cRegAddr, &cRegVal, 1);

	return cRegVal;
}

static uint8_t Si5351_ReadField(uint8_t cRegAddr, uint8_t cMask, uint8_t cShift)
{
	uint8_t cRegValue = Si5351_ReadReg(cRegAddr);

	return (cRegValue & cMask) >> cShift;
}

uint8_t cRegValue;
static uint8_t Si5351_WriteField(uint8_t cRegAddr, uint8_t cMask, uint8_t cShift, uint8_t cValue)
{
	uint8_t acBuffer[3];

	acBuffer[0] = SI5351_ADDR;
	acBuffer[1] = cRegAddr;

	if (cMask == 0xFF)
	{
		acBuffer[2] = cValue;
	}
	else
	{
		cRegValue = Si5351_ReadReg(cRegAddr);
		cRegValue = (cRegValue & ~cMask) | (cValue << cShift);
		acBuffer[2] = cRegValue;
	}

	return (I2CWrite(acBuffer, 3) == 3)? 1 : 0;
}

// ----------------------------------------------------------------------------
// 										API
// ----------------------------------------------------------------------------
uint8_t debugVar = 0;
void Si5351_Initialize(void)
{
	uint16_t i;

	for (i=0; i<sizeof(m_acRegInit); i+=2)
	{
		if (!Si5351_WriteField(m_acRegInit[i], 0xFF, 0, m_acRegInit[i+1]))
		{
			debugVar = 1;
		}
	}

	cRegValue = Si5351_ReadReg(0);
}

void Si5351_OutputEnable(uint8_t bEnable)
{
	if (bEnable == 0)
	{
		Si5351_WriteField(OUT_EN_CTRL_REG, 0xFF, 0, 0xFF);
		SI5351_WRITE_FIELD(CLK0_CTRL_REG, CLK0_PDN, 1);
		SI5351_WRITE_FIELD(CLK1_CTRL_REG, CLK1_PDN, 1);
	}
	else
	{
		Si5351_WriteField(OUT_EN_CTRL_REG, 0xFF, 0, 0xFC);
		SI5351_WRITE_FIELD(CLK0_CTRL_REG, CLK0_PDN, 0);
		SI5351_WRITE_FIELD(CLK1_CTRL_REG, CLK1_PDN, 0);
	}
}

void Si5351_PLLReset(void)
{
	Si5351_WriteField(PLL_RESET_REG, 0xff, 0, 0xAC);
}

void Si5351_OutputFrequency(uint32_t dwFreqHz)
{
	uint32_t P1;
	uint32_t P2;
	uint32_t P3;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	unsigned long long ddwRem;

	Si5351_OutputEnable(0);

	a = F_VCO/dwFreqHz;
	c = 1048575;	//2^20-1
	ddwRem = F_VCO - a*dwFreqHz;
	b = (uint32_t)((c*ddwRem+dwFreqHz/2)/dwFreqHz);

	P1 = 128*a + 128*b/c - 512;
	P2 = 128*b - c*(128*b/c);
	P3 = c;

	SI5351_WRITE_FIELD(MS0_PAR_44_REG, MS0_P1_17_16, (P1>>16) & 3);
	SI5351_WRITE_FIELD(MS0_PAR_45_REG, MS0_P1_15_8, (P1>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS0_PAR_46_REG, MS0_P1_7_0, P1 & 0xFF);

	SI5351_WRITE_FIELD(MS0_PAR_47_REG, MS0_P2_19_16, (P2>>16) & 0x0F);
	SI5351_WRITE_FIELD(MS0_PAR_48_REG, MS0_P2_15_8, (P2>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS0_PAR_49_REG, MS0_P2_7_0, P2 & 0xFF);

	SI5351_WRITE_FIELD(MS0_PAR_47_REG, MS0_P3_19_16, (P3>>16) & 0x0F);
	SI5351_WRITE_FIELD(MS0_PAR_42_REG, MS0_P3_15_8, (P3>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS0_PAR_43_REG, MS0_P3_7_0, P3 & 0xFF);

	SI5351_WRITE_FIELD(MS1_PAR_52_REG, MS1_P1_17_16, (P1>>16) & 3);
	SI5351_WRITE_FIELD(MS1_PAR_53_REG, MS1_P1_15_8, (P1>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS1_PAR_54_REG, MS1_P1_7_0, P1 & 0xFF);

	SI5351_WRITE_FIELD(MS1_PAR_55_REG, MS1_P2_19_16, (P2>>16) & 0x0F);
	SI5351_WRITE_FIELD(MS1_PAR_56_REG, MS1_P2_15_8, (P2>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS1_PAR_57_REG, MS1_P2_7_0, P2 & 0xFF);

	SI5351_WRITE_FIELD(MS1_PAR_55_REG, MS1_P3_19_16, (P3>>16) & 0x0F);
	SI5351_WRITE_FIELD(MS1_PAR_50_REG, MS1_P3_15_8, (P3>>8) & 0xFF);
	SI5351_WRITE_FIELD(MS1_PAR_51_REG, MS1_P3_7_0, P3 & 0xFF);

	Si5351_OutputEnable(1);
	Si5351_PLLReset();
}

