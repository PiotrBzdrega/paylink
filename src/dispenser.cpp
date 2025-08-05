#include "dispenser.h"

namespace paylink
{
            //     /*-- Determine unit type ------------------------------------------*/
            // switch (Dispenser.Unit)
            // {
            // case DP_AS_WH2:
            //     DispenserName = "Asahi Seiko Escalator";
            //     break;

            // case DP_AZK_HOPPER:
            //     DispenserName = "Azkoyen Hopper";
            //     break;

            // case DP_AZK_HOPPER_U:
            //     DispenserName = "Azkoyen Hopper U";
            //     break;

            // case DP_AZK_HOPPER_UPL:
            //     DispenserName = "Azkoyen Hopper U+";
            //     break;

            // case DP_CASHLESS_HUB:
            //     DispenserName = "Cashless Hub";
            //     break;

            // case DP_CC_CASSETTE:
            //     DispenserName = "Bill to Bill Cassette";
            //     break;

            // case DP_CDM_4000:
            //     DispenserName = "MFS CDM 4000 Cassette";
            //     break;

            // case DP_CLS_HOPPER:
            //     DispenserName = "CLS Hopper";
            //     break;

            // case DP_CX25_TUBE:
            //     DispenserName = "Telequip CX25 Hopper";
            //     break;

            // case DP_EBDS_ROLL:
            //     DispenserName = "EBDS Recycler Roll";
            //     break;

            // case DP_FUJITSU_F400:
            //     DispenserName = "Fujitsu F4000";
            //     break;

            // case DP_FUJITSU_F53:
            //     DispenserName = "Fujitsu F53";
            //     break;

            // case DP_FUJITSU_F56:
            //     DispenserName = "Fujitsu F56";
            //     break;

            // case DP_ID003_BOX:
            //     DispenserName = "ID003 Recycler Note Box";
            //     break;

            // case DP_INNOV_NV11_RC:
            //     DispenserName = "NV11 Note Recycler";
            //     break;

            // case DP_INNOV_NV200_NOTE:
            //     DispenserName = "NV200 Denomination";
            //     break;

            // case DP_JCM_VEGA_RC:
            //     DispenserName = "JCM Vega Note";
            //     break;

            // case DP_MCL_BCR_HOPPER:
            //     DispenserName = "MCL BCR Hopper";
            //     break;

            // case DP_MCL_CR100_HOPPER:
            //     DispenserName = "MCL CR10x Hopper";
            //     break;

            // case DP_MCL_NR2_HOPPER:
            //     DispenserName = "MCL NR2 Hopper";
            //     break;

            // case DP_CC_GHOST_HOPPER:
            //     DispenserName = "Ghost (Coin count only)";
            //     break;

            // case DP_MCL_SCH2:
            //     DispenserName = "MCL Serial Compact Hopper 2";
            //     break;

            // case DP_MCL_SCH3:
            //     DispenserName = "MCL Serial Compact Hopper 3";
            //     break;

            // case DP_MCL_SCH3A:
            //     DispenserName = "MCL Combi Hopper";
            //     break;

            // case DP_MCL_SCH5:
            //     DispenserName = "MCL Compact Hopper 5 (DES)";
            //     break;

            // case DP_MCL_SUH1:
            //     DispenserName = "MCL Serial Universal Hopper";
            //     break;

            // case DP_MCL_SUH5:
            //     DispenserName = "MCL Universal Hopper 5 (DES)";
            //     break;

            // case DP_MEIBNR_LOADER:
            //     DispenserName = "Mars BNR Loader";
            //     break;

            // case DP_MEIBNR_RECYCLER:
            //     DispenserName = "Mars BNR Recycler";
            //     break;

            // case DP_MDB_LEVEL_2_TUBE:
            //     DispenserName = "MDB Tube";
            //     break;

            // case DP_MDB_TYPE_3_PAYOUT:
            //     DispenserName = "MDB Payout System";
            //     break;

            // case DP_MERKUR_100_PAY:
            //     DispenserName = "Merkur MD100 Roll";
            //     break;

            // case DP_NRI_CURRENZA_H2:
            //     DispenserName = "NRI Currenza H2";
            //     break;

            // case DP_SHOPPER:
            //     DispenserName = "Innovative SmartHopper";
            //     break;

            // case DP_SHOPPER_TOTAL:
            //     DispenserName = "Innovative Smart Hopper Summary";
            //     break;

            // case DP_TFLEX_TUBE:
            //     DispenserName = "Telequip TFlex Tube";
            //     break;

            // default:
            //     sprintf(Buffer, "Unknown code %08x", Dispenser.Unit);
            //     DispenserName = Buffer;
            //     break;
            // }

            // /*-- Determine coin count contents --------------------------------*/
            // switch (Dispenser.CoinCountStatus)
            // { case DISPENSER_COIN_NONE    : strcpy (buff, "-");                             break;
            //   case DISPENSER_COIN_LOW     : strcpy (buff, "Low");                           break;
            //   case DISPENSER_COIN_MID     : strcpy (buff, "Normal");                        break;
            //   case DISPENSER_COIN_HIGH    : strcpy (buff, "High");                          break;
            //   case DISPENSER_ACCURATE     : sprintf(buff, "%d",      Dispenser.CoinCount); break;
            //   case DISPENSER_ACCURATE_FULL: sprintf(buff, "Full %d", Dispenser.CoinCount); break;
            //   default                     : strcpy (buff, "Unknown");                       break;
            // }

            // /*-- Determine dispenser status -----------------------------------*/
            // switch (Dispenser.Status)
            // { case PAY_FINISHED      : strcpy(buff, "Idle OK");         break;
            //   case PAY_ONGOING       : strcpy(buff, "Paying");          break;
            //   case PAY_EMPTY         : strcpy(buff, "Empty");           break;
            //   case PAY_JAMMED        : strcpy(buff, "Jammed");          break;
            //   case PAY_US            : strcpy(buff, "U/S");             break;
            //   case PAY_FRAUD         : strcpy(buff, "Fraud Attempt");   break;
            //   case PAY_FAILED_BLOCKED: strcpy(buff, "Blocked");         break;
            //   case PAY_NO_HOPPER     : strcpy(buff, "No Dispenser");    break;
            //   case PAY_INHIBITED     : strcpy(buff, "Inhibited");       break;
            //   case PAY_SECURITY_FAIL : strcpy(buff, "Security Fail");   break;
            //   case PAY_HOPPER_RESET  : strcpy(buff, "Dispenser Reset"); break;
            //   case PAY_NOT_EXACT     : strcpy(buff, "No Exact Coin");   break;
            //   case PAY_GHOST         : strcpy(buff, "Ghost");           break;
            //   default                : sprintf(buff, "Error %d", Dispenser.Status);
            // }

            // /*-- Determine if the dispenser is inhibited ----------------------*/
            // if(Dispenser.Inhibit == 0)
            // {
            //     std::println("Dispenser Inhibited");
            // }
}