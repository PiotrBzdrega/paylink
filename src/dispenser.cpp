#include "dispenser.h"
#include "logger.h"
#include <print>
#include <format>

namespace paylink
{

    std::string_view dispenser::unitToString()
    {
        /*-- Determine unit type ------------------------------------------*/
        switch (block.Unit)
        {
        case DP_AS_WH2:
            return "Asahi Seiko Escalator";
            break;

        case DP_AZK_HOPPER:
            return "Azkoyen Hopper";
            break;

        case DP_AZK_HOPPER_U:
            return "Azkoyen Hopper U";
            break;

        case DP_AZK_HOPPER_UPL:
            return "Azkoyen Hopper U+";
            break;

        case DP_CASHLESS_HUB:
            return "Cashless Hub";
            break;

        case DP_CC_CASSETTE:
            return "Bill to Bill Cassette";
            break;

        case DP_CDM_4000:
            return "MFS CDM 4000 Cassette";
            break;

        case DP_CLS_HOPPER:
            return "CLS Hopper";
            break;

        case DP_CX25_TUBE:
            return "Telequip CX25 Hopper";
            break;

        case DP_EBDS_ROLL:
            return "EBDS Recycler Roll";
            break;

        case DP_FUJITSU_F400:
            return "Fujitsu F4000";
            break;

        case DP_FUJITSU_F53:
            return "Fujitsu F53";
            break;

        case DP_FUJITSU_F56:
            return "Fujitsu F56";
            break;

        case DP_ID003_BOX:
            return "ID003 Recycler Note Box";
            break;

        case DP_INNOV_NV11_RC:
            return "NV11 Note Recycler";
            break;

        case DP_INNOV_NV200_NOTE:
            return "NV200 Denomination";
            break;

        case DP_JCM_VEGA_RC:
            return "JCM Vega Note";
            break;

        case DP_MCL_BCR_HOPPER:
            return "MCL BCR Hopper";
            break;

        case DP_MCL_CR100_HOPPER:
            return "MCL CR10x Hopper";
            break;

        case DP_MCL_NR2_HOPPER:
            return "MCL NR2 Hopper";
            break;

        case DP_CC_GHOST_HOPPER:
            return "Ghost (Coin count only)";
            break;

        case DP_MCL_SCH2:
            return "MCL Serial Compact Hopper 2";
            break;

        case DP_MCL_SCH3:
            return "MCL Serial Compact Hopper 3";
            break;

        case DP_MCL_SCH3A:
            return "MCL Combi Hopper";
            break;

        case DP_MCL_SCH5:
            return "MCL Compact Hopper 5 (DES)";
            break;

        case DP_MCL_SUH1:
            return "MCL Serial Universal Hopper";
            break;

        case DP_MCL_SUH5:
            return "MCL Universal Hopper 5 (DES)";
            break;

        case DP_MEIBNR_LOADER:
            return "Mars BNR Loader";
            break;

        case DP_MEIBNR_RECYCLER:
            return "Mars BNR Recycler";
            break;

        case DP_MDB_LEVEL_2_TUBE:
            return "MDB Tube";
            break;

        case DP_MDB_TYPE_3_PAYOUT:
            return "MDB Payout System";
            break;

        case DP_MERKUR_100_PAY:
            return "Merkur MD100 Roll";
            break;

        case DP_NRI_CURRENZA_H2:
            return "NRI Currenza H2";
            break;

        case DP_SHOPPER:
            return "Innovative SmartHopper";
            break;

        case DP_SHOPPER_TOTAL:
            return "Innovative Smart Hopper Summary";
            break;

        case DP_TFLEX_TUBE:
            return "Telequip TFlex Tube";
            break;

        default:
            return std::format("Unknown code {:08x}", block.Unit);
            break;
        }
    }

    std::string_view dispenser::coinLevelToString()
    {

        /*-- Determine coin count contents --------------------------------*/
        switch (block.CoinCountStatus)
        {
        case DISPENSER_COIN_NONE:
            return "-";
            break;
        case DISPENSER_COIN_LOW:
            return "Low";
            break;
        case DISPENSER_COIN_MID:
            return "Normal";
            break;
        case DISPENSER_COIN_HIGH:
            return "High";
            break;
        case DISPENSER_ACCURATE:
            return std::format("{}", block.CoinCount);
            break;
        case DISPENSER_ACCURATE_FULL:
            return std::format("Full {}", block.CoinCount);
            break;
        default:
            return "Unknown";
            break;
        }
    }

    std::string_view dispenser::statusToString()
    {
        /*-- Determine dispenser status -----------------------------------*/
        switch (block.Status)
        {
        case PAY_FINISHED:
            return "Idle OK";
            break;
        case PAY_ONGOING:
            return "Paying";
            break;
        case PAY_EMPTY:
            return "Empty";
            break;
        case PAY_JAMMED:
            return "Jammed";
            break;
        case PAY_US:
            return "U/S";
            break;
        case PAY_FRAUD:
            return "Fraud Attempt";
            break;
        case PAY_FAILED_BLOCKED:
            return "Blocked";
            break;
        case PAY_NO_HOPPER:
            return "No Dispenser";
            break;
        case PAY_INHIBITED:
            return "Inhibited";
            break;
        case PAY_SECURITY_FAIL:
            return "Security Fail";
            break;
        case PAY_HOPPER_RESET:
            return "Dispenser Reset";
            break;
        case PAY_NOT_EXACT:
            return "No Exact Coin";
            break;
        case PAY_GHOST:
            return "Ghost";
            break;
        default:
            return std::format("Error {}", block.Status);
        }
    }

    bool dispenser::init()
    {
        size_t dispenser_no{};
        for (; ReadDispenserDetails(dispenser_no, operator&()); ++dispenser_no)
        {
            mik::logger::debug("index {}", dispenser_no);
            debug_info();
            // setInhibit(false);
            block.Inhibit = 0; //uninhibited
            WriteDispenserDetails(dispenser_no, operator&());
        }
        return (init_ok = static_cast<bool>(dispenser_no));
    }

    void dispenser::debug_info()
    {
        mik::logger::debug("Unit: {}", unitToString());
        mik::logger::debug("Status: {}", statusToString());
        mik::logger::debug("InterfaceNumber: {}", block.InterfaceNumber);
        mik::logger::debug("UnitAddress: {}", block.UnitAddress);
        mik::logger::debug("Coin level: {}", block.Count);
        mik::logger::debug("Coin value: {}", block.Value);
        mik::logger::debug("Coin level: {}", coinLevelToString());
        mik::logger::debug("Inhibited: {}", block.Inhibit);
        mik::logger::debug("Description: {}", block.Description); // TODO: crash
        mik::logger::debug("SerialNumber: {}", block.SerialNumber);
    }

    // /*-- Determine if the dispenser is inhibited ----------------------*/
    // if(Dispenser.Inhibit == 0)
    // {
    //     mik::logger::debug("Dispenser Inhibited");
    // }
}