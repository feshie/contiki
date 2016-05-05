/* Modulation format = 2-GFSK */
/* Address config = No address check */
/* Packet bit length = 0 */
/* Packet length = 255 */
/* Symbol rate = 50 */
/* Device address = 0 */
/* Bit rate = 50 */
/* RX filter BW = 104.166667 */
/* Whitening = false */
/* Packet length mode = Variable */
/* Deviation = 24.948120 */
/* Carrier frequency = 862.999878 */
/* Manchester enable = false */
/* CC1200 settings exported from SmartRF as a function 
 * that is called by the CC1120 Contiki driver. 
 * 
 * Alternatives to this file can be created in SmartRF by 
 * importing srfexp_c-command.xml or re-creating the template.  
 *
 * The Template is filled out as follows:
 *  "File" contains cc1200-config.c
 *  "Comment" contains C-Comment initiator and terminator
 *  "Parameter Summary" box is ticked
 *  "Header" contains all text from * CC1120 setttings... to opening {
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
cc1200_register_config(void)
{
   cc1120_spi_single_write(CC1200_ADDR_IOCFG2, 0x06);          // GPIO2 IO Pin Configuration.
   cc1120_spi_single_write(CC1200_ADDR_SYNC3, 0x7A);           // Sync Word Configuration [31:24].
   cc1120_spi_single_write(CC1200_ADDR_SYNC2, 0x0E);           // Sync Word Configuration [23:16].
   cc1120_spi_single_write(CC1200_ADDR_SYNC1, 0x90);           // Sync Word Configuration [15:8].
   cc1120_spi_single_write(CC1200_ADDR_SYNC0, 0x4E);           // Sync Word Configuration [7:0].
   cc1120_spi_single_write(CC1200_ADDR_SYNC_CFG1, 0xE5);       // Sync Word Detection Configuration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_SYNC_CFG0, 0x23);       // Sync Word Detection Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_DEVIATION_M, 0x47);     // Frequency Deviation Configuration.
   cc1120_spi_single_write(CC1200_ADDR_MODCFG_DEV_E, 0x0B);    // Modulation Format and Frequency Deviation Configur...
   cc1120_spi_single_write(CC1200_ADDR_DCFILT_CFG, 0x56);      // Digital DC Removal Configuration.
   cc1120_spi_single_write(CC1200_ADDR_PREAMBLE_CFG1, 0x18);   // Preamble Length Configuration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_PREAMBLE_CFG0, 0xBA);   // Preamble Detection Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_IQIC, 0xC8);            // Digital Image Channel Compensation Configuration.
   cc1120_spi_single_write(CC1200_ADDR_CHAN_BW, 0x84);         // Channel Filter Configuration.
   cc1120_spi_single_write(CC1200_ADDR_MDMCFG1, 0x42);         // General Modem Parameter Configuration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_MDMCFG0, 0x05);         // General Modem Parameter Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_SYMBOL_RATE2, 0x94);    // Symbol Rate Configuration Exponent and Mantissa [1...
   cc1120_spi_single_write(CC1200_ADDR_SYMBOL_RATE1, 0x7A);    // Symbol Rate Configuration Mantissa [15:8].
   cc1120_spi_single_write(CC1200_ADDR_SYMBOL_RATE0, 0xE1);    // Symbol Rate Configuration Mantissa [7:0].
   cc1120_spi_single_write(CC1200_ADDR_AGC_REF, 0x27);         // AGC Reference Level Configuration.
   cc1120_spi_single_write(CC1200_ADDR_AGC_CS_THR, 0xF1);      // Carrier Sense Threshold Configuration.
   cc1120_spi_single_write(CC1200_ADDR_AGC_CFG1, 0x11);        // Automatic Gain Control Configuration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_AGC_CFG0, 0x90);        // Automatic Gain Control Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_FIFO_CFG, 0x80);        // FIFO Configuration.
   cc1120_spi_single_write(CC1200_ADDR_FS_CFG, 0x12);          // Frequency Synthesizer Configuration.
   cc1120_spi_single_write(CC1200_ADDR_PKT_CFG0, 0x20);        // Packet Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_PA_CFG0, 0x53);         // Power Amplifier Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_PKT_LEN, 0xFF);         // Packet Length Configuration.
   cc1120_spi_single_write(CC1200_ADDR_IF_MIX_CFG, 0x18);      // IF Mix Configuration.
   cc1120_spi_single_write(CC1200_ADDR_TOC_CFG, 0x03);         // Timing Offset Correction Configuration.
   cc1120_spi_single_write(CC1200_ADDR_MDMCFG2, 0x02);         // General Modem Parameter Configuration Reg. 2.
   cc1120_spi_single_write(CC1200_ADDR_FREQ2, 0x56);           // Frequency Configuration [23:16].
   cc1120_spi_single_write(CC1200_ADDR_FREQ1, 0x4C);           // Frequency Configuration [15:8].
   cc1120_spi_single_write(CC1200_ADDR_FREQ0, 0xCC);           // Frequency Configuration [7:0].
   cc1120_spi_single_write(CC1200_ADDR_IF_ADC1, 0xEE);         // Analog to Digital Converter Configuration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_IF_ADC0, 0x10);         // Analog to Digital Converter Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_FS_DIG1, 0x04);         // Frequency Synthesizer Digital Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_FS_DIG0, 0x50);         // Frequency Synthesizer Digital Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_FS_CAL1, 0x40);         // Frequency Synthesizer Calibration Reg. 1.
   cc1120_spi_single_write(CC1200_ADDR_FS_CAL0, 0x0E);         // Frequency Synthesizer Calibration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_FS_DIVTWO, 0x03);       // Frequency Synthesizer Divide by 2.
   cc1120_spi_single_write(CC1200_ADDR_FS_DSM0, 0x33);         // FS Digital Synthesizer Module Configuration Reg. 0.
   cc1120_spi_single_write(CC1200_ADDR_FS_DVC1, 0xF7);         // Frequency Synthesizer Divider Chain Configuration ...
   cc1120_spi_single_write(CC1200_ADDR_FS_DVC0, 0x0F);         // Frequency Synthesizer Divider Chain Configuration ...
   cc1120_spi_single_write(CC1200_ADDR_FS_PFD, 0x00);          // Frequency Synthesizer Phase Frequency Detector Con...
   cc1120_spi_single_write(CC1200_ADDR_FS_PRE, 0x6E);          // Frequency Synthesizer Prescaler Configuration.
   cc1120_spi_single_write(CC1200_ADDR_FS_REG_DIV_CML, 0x1C);  // Frequency Synthesizer Divider Regulator Configurat...
   cc1120_spi_single_write(CC1200_ADDR_FS_SPARE, 0xAC);        // Frequency Synthesizer Spare.
   cc1120_spi_single_write(CC1200_ADDR_FS_VCO0, 0xB5);         // FS Voltage Controlled Oscillator Configuration Reg...
   cc1120_spi_single_write(CC1200_ADDR_IFAMP, 0x05);           // Intermediate Frequency Amplifier Configuration.
   cc1120_spi_single_write(CC1200_ADDR_XOSC5, 0x0E);           // Crystal Oscillator Configuration Reg. 5.
   cc1120_spi_single_write(CC1200_ADDR_XOSC1, 0x03);           // Crystal Oscillator Configuration Reg. 1.
}
