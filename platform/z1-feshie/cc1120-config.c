/* Deviation = 24.963379 */
/* PA ramping = true */
/* RX filter BW = 100.000000 */
/* Address config = No address check */
/* Packet length = 255 */
/* Packet length mode = Variable */
/* Bit rate = 50 */
/* Modulation format = 2-GFSK */
/* Performance mode = High Performance */
/* Symbol rate = 50 */
/* Packet bit length = 0 */
/* Whitening = false */
/* Device address = 0 */
/* Carrier frequency = 868.000000 */
/* Manchester enable = false */
/* TX power = 15 */

/* CC1120 settings exported from SmartRF as a function 
 * that is called by the CC1120 Contiki driver. 
 * 
 * Alternatives to this file can be created in SmartRF by 
 * importing srfexp_c-command.xml or re-creating the template.  
 *
 * The Template is filled out as follows:
 *  "File" contains cc1120-config.c
 *  "Comment" contains C-Comment initiator and terminator
 *  "Parameter Summary" box is ticked
 *  "Header" contains all text from "/* CC1120 setttings..." to opening {
 *  "Registers" contains the formatting string "   cc1120_spi_single_write(CC1120_ADDR_@RN@, 0x@VH@); @<<@ // @Rd@."
 *  "Footer" contains a closing brace }
 *
 * Please note: Some exported settings are over-written by 
 * other parts of the code (such as FREQ, which is set as 
 * part of the set channel function, and GPIO configuration,  
 * which is controlled by settings in the platform-conf.h.
 */

#include "cc1120-config.h"
#include "cc1120.h"
 
void
cc1120_register_config(void)
{
   cc1120_spi_single_write(CC1120_ADDR_IOCFG3, 0xB0);          /* GPIO3 IO Pin Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_IOCFG2, 0x06);          /* GPIO2 IO Pin Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_IOCFG1, 0xB0);          /* GPIO1 IO Pin Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_IOCFG0, 0x40);          /* GPIO0 IO Pin Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_SYNC_CFG1, 0x08);       /* Sync Word Detection Configuration Reg. 1. */
   cc1120_spi_single_write(CC1120_ADDR_DEVIATION_M, 0x99);     /* Frequency Deviation Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_MODCFG_DEV_E, 0x0D);    /* Modulation Format and Frequency Deviation Configur... */
   cc1120_spi_single_write(CC1120_ADDR_DCFILT_CFG, 0x15);      /* Digital DC Removal Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_PREAMBLE_CFG1, 0x18);   /* Preamble Length Configuration Reg. 1. */
   cc1120_spi_single_write(CC1120_ADDR_FREQ_IF_CFG, 0x3A);     /* RX Mixer Frequency Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_IQIC, 0x00);            /* Digital Image Channel Compensation Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_CHAN_BW, 0x02);         /* Channel Filter Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_MDMCFG0, 0x05);         /* General Modem Parameter Configuration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_SYMBOL_RATE2, 0x99);    /* Symbol Rate Configuration Exponent and Mantissa [1... */
   cc1120_spi_single_write(CC1120_ADDR_SYMBOL_RATE1, 0x99);    /* Symbol Rate Configuration Mantissa [15:8]. */
   cc1120_spi_single_write(CC1120_ADDR_SYMBOL_RATE0, 0x99);    /* Symbol Rate Configuration Mantissa [7:0]. */
   cc1120_spi_single_write(CC1120_ADDR_AGC_REF, 0x3C);         /* AGC Reference Level Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_AGC_CS_THR, 0xEF);      /* Carrier Sense Threshold Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_AGC_CFG1, 0xA9);        /* Automatic Gain Control Configuration Reg. 1. */
   cc1120_spi_single_write(CC1120_ADDR_AGC_CFG0, 0xC0);        /* Automatic Gain Control Configuration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_FIFO_CFG, 0x00);        /* FIFO Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_SETTLING_CFG, 0x03);    /* Frequency Synthesizer Calibration and Settling Con... */
   cc1120_spi_single_write(CC1120_ADDR_FS_CFG, 0x12);          /* Frequency Synthesizer Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_PKT_CFG0, 0x20);        /* Packet Configuration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_PA_CFG0, 0x79);         /* Power Amplifier Configuration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_PKT_LEN, 0xFF);         /* Packet Length Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_IF_MIX_CFG, 0x00);      /* IF Mix Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_TOC_CFG, 0x0A);         /* Timing Offset Correction Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_FREQ2, 0x6C);           /* Frequency Configuration [23:16]. */
   cc1120_spi_single_write(CC1120_ADDR_FREQ1, 0x80);           /* Frequency Configuration [15:8]. */
   cc1120_spi_single_write(CC1120_ADDR_FS_DIG1, 0x00);         /* Frequency Synthesizer Digital Reg. 1. */
   cc1120_spi_single_write(CC1120_ADDR_FS_DIG0, 0x5F);         /* Frequency Synthesizer Digital Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_FS_CAL1, 0x40);         /* Frequency Synthesizer Calibration Reg. 1. */
   cc1120_spi_single_write(CC1120_ADDR_FS_CAL0, 0x0E);         /* Frequency Synthesizer Calibration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_FS_DIVTWO, 0x03);       /* Frequency Synthesizer Divide by 2. */
   cc1120_spi_single_write(CC1120_ADDR_FS_DSM0, 0x33);         /* FS Digital Synthesizer Module Configuration Reg. 0. */
   cc1120_spi_single_write(CC1120_ADDR_FS_DVC0, 0x17);         /* Frequency Synthesizer Divider Chain Configuration ... */
   cc1120_spi_single_write(CC1120_ADDR_FS_PFD, 0x50);          /* Frequency Synthesizer Phase Frequency Detector Con... */
   cc1120_spi_single_write(CC1120_ADDR_FS_PRE, 0x6E);          /* Frequency Synthesizer Prescaler Configuration. */
   cc1120_spi_single_write(CC1120_ADDR_FS_REG_DIV_CML, 0x14);  /* Frequency Synthesizer Divider Regulator Configurat... */
   cc1120_spi_single_write(CC1120_ADDR_FS_SPARE, 0xAC);        /* Frequency Synthesizer Spare. */
   cc1120_spi_single_write(CC1120_ADDR_FS_VCO0, 0xB4);         /* FS Voltage Controlled Oscillator Configuration Reg... */
   cc1120_spi_single_write(CC1120_ADDR_XOSC5, 0x0E);           /* Crystal Oscillator Configuration Reg. 5. */
   cc1120_spi_single_write(CC1120_ADDR_XOSC1, 0x03);           /* Crystal Oscillator Configuration Reg. 1. */
}
