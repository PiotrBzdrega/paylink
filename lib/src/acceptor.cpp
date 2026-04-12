#include "acceptor.h"
#include "logger.h"

namespace paylink
{
    void acceptor::debug_info()
    {
        mik::logger::debug("Unit: {}", unitToString());
        mik::logger::debug("Status: {}", statusToString());
        mik::logger::debug("InterfaceNumber: {}", block.InterfaceNumber);
        mik::logger::debug("UnitAddress: {}", block.UnitAddress);
        mik::logger::debug("Currency: {}", block.Currency);
        mik::logger::debug("DefaultPath: {}", block.DefaultPath);
        mik::logger::debug("NoOfCoins: {}", block.NoOfCoins);
        // mik::logger::debug("Description: {}", block.Description); //TODO: crash
        mik::logger::debug("SerialNumber: {}", block.SerialNumber);
    }

    void acceptor::setInhibit(int index, bool state)
    {
        auto is_inhibit = static_cast<bool>(block.Status & ACCEPTOR_INHIBIT);

        if (state && !is_inhibit)
        {
            block.Status |= ACCEPTOR_INHIBIT;
        }
        else if (!state && is_inhibit)
        {
            block.Status &= ~ACCEPTOR_INHIBIT;
        }
        WriteAcceptorDetails(index, &block);
    }

    std::string_view acceptor::unitToString()
    {
        switch (block.Unit)
        {
        case DP_AST_GBA:
            return "AstroSystems GBA";
            break;

        case DP_AZK_A6:
            return "Azkoyen A6";
            break;

        case DP_AZK_MDB:
            return "Azkoyen MDB Changer";
            break;

        case DP_AZK_X6:
            return "Azkoyen X6";
            break;

        case DP_CC_ACCEPTOR:
            return "CashCode Acceptor";
            break;

        case DP_CC_RECYCLER:
            return "Cashcode Recycler";
            break;

        case DP_CLS:
            return "Crane PI CLS";
            break;

        case DP_COINCO_BILLPRO:
            return "CoinCo BillPro";
            break;

        case DP_COINCO_GLOBAL:
            return "CoinCo Global";
            break;

        case DP_COINCO_MDB:
            return "CoinCo MDB Changer";
            break;

        case DP_COINCO_MDB_BILL:
            return "CoinCo MDB Bill";
            break;

        case DP_COINCO_VORTEX:
            return "CoinCo Vortex";
            break;

        case DP_EBDS_NOTE:
            return "EBDS";
            break;

        case DP_EBDS_RECYCLER:
            return "EBDS Recycler";
            break;

        case DP_GPT_NOTE:
            return "GPT Argus";
            break;

        case DP_ICT_U85:
            return "ICT U85";
            break;

        case DP_ID003_NOTE:
            return "ID-003 Note";
            break;

        case DP_ID003_RECYCLER:
            return "ID-003 Note Recycler";
            break;

        case DP_INNOV_NV10:
            return "Innovative NV10";
            break;

        case DP_INNOV_NV11:
            return "Innovative NV11";
            break;

        case DP_INNOV_NV200:
            return "Innovative NV200";
            break;

        case DP_INNOV_NV4:
            return "Innovative NV4";
            break;

        case DP_INNOV_NV7:
            return "Innovative NV7";
            break;

        case DP_INNOV_NV8:
            return "Innovative NV8";
            break;

        case DP_INNOV_NV9:
            return "Innovative NV9";
            break;

        case DP_JCM_CC_EBA:
            return "JCM EBA on cctalk";
            break;

        case DP_JCM_CC_WBA:
            return "JCM WBA on cctalk";
            break;

        case DP_JCM_NOTE:
            return "JCM Note";
            break;

        case DP_JCM_VEGA:
            return "JCM Vega";
            break;

        case DP_MARS_CASHFLOW_126:
            return "Mars Cashflow 126";
            break;

        case DP_MARS_CASHFLOW_690:
            return "Mars Cashflow 690";
            break;

        case DP_MARS_CASHFLOW_9500:
            return "Mars Cashflow 9500";
            break;

        case DP_MARS_MDB:
            return "Mars MDB Changer";
            break;

        case DP_MARS_SCR_ADVANCE:
            return "SCR Advance Recycler";
            break;

        case DP_MARS_SC_ADVANCE:
            return "SC Advance";
            break;
        case DP_MCL_7200:
            return "MCL 7200";
            break;

        case DP_MCL_ARDAC:
            return "MCL Ardac /ID-003";
            break;

        case DP_MCL_ARDAC_ELITE:
            return "MCL Ardac Elite";
            break;

        case DP_MCL_BCR:
            return "MCL BCR Coin Recycler";
            break;

        case DP_MCL_BCS:
            return "MCL Bulk coin Sorter";
            break;

        case DP_MCL_CONDOR:
            return "MCL Condor";
            break;

        case DP_MCL_CR100:
            return "MCL CR 10x Recycler";
            break;

        case DP_MCL_LUMINA:
            return "MCL Lumina";
            break;

        case DP_MCL_NR2:
            return "MCL NR2 recycler";
            break;

        case DP_MCL_SR3:
            return "MCL SR3";
            break;

        case DP_MCL_SR5:
            return "MCL SR5";
            break;

        case DP_MCL_WACS:
            return "MCL Ardac / WACS";
            break;

        case DP_MDB_BILL:
            return "MBD Bill";
            break;

        case DP_MDB_LEVEL_2:
            return "MDB Level 2 Changer";
            break;

        case DP_MDB_LEVEL_3:
            return "MDB Level 3 Changer";
            break;

        case DP_MEIBNR:
            return "MEI BNR Recycler";
            break;

        case DP_MERKUR_100:
            return "Merkur MD100 Recycler";
            break;

        case DP_NRI_EAGLE:
            return "NRI Eagle";
            break;

        case DP_NRI_G40:
            return "NRI G40";
            break;

        case DP_NRI_PELICANO:
            return "NRI Pelicano";
            break;

        case DP_SHOPPER_ACCEPT:
            return "SmartHopper Acceptor";
            break;

        case DP_SSP_NOTE:
            return "SSP Acceptor";
            break;

        case DP_SSP_RECYCLER:
            return "SSP Recycler";
            break;

        case DP_CCTALK_INTERFACE | DP_COIN_ACCEPT_DEVICE:
            return "ccTalk Coin";
            break;

        case DP_CCTALK_INTERFACE | DP_NOTE_ACCEPT_DEVICE:
            return "ccTalk Note";
            break;

        default:
            return std::format("Unknown code {:08x}", block.Unit);
            break;
        }
    }
    std::string_view acceptor::statusToString()
    {
        switch (block.Status)
        {
        case 0:
            return "OK";
            break;
        case ACCEPTOR_DEAD:
            return "Dead";
            break;
        // case ACCEPTOR_INHIBIT : strcpy(buff, "Inhibit");              break; //Specified by app
        case ACCEPTOR_DISABLED:
            return "Disabled";
            break;
        case ACCEPTOR_FRAUD:
            return "Fraud";
            break;
        case ACCEPTOR_BUSY:
            return "Busy";
            break;
        case ACCEPTOR_FAULT:
            return "Fault";
            break;
        case ACCEPTOR_NO_KEY:
            return "No Key";
            break;
        default:
            return std::format("{:08x}", block.Status);
            break;
        }
    }

    bool acceptor::init()
    {
        /*
            main
            create_win_acceptors
            on_win_acceptors_show
            DataFn_StartAcceptorTimer - starts timer that calls "DataFn_AcceptorUpdateData" every 500ms
            DataFn_AcceptorUpdateData
        */
        /* The sequence numbers of the acceptors are contiguous and run from zero upwards. */
        /* The ReadAcceptorDetails call provides a snapshot of all the information
        possessed by the interface on a single unit of money handling equipment. */
        // TODO: why first call gives status Disabled ??
        size_t acceptor_no{};
        for (; ReadAcceptorDetails(acceptor_no, &block /* operator&() */); ++acceptor_no)
        {
            mik::logger::debug("index {}", acceptor_no);
            debug_info();
            setInhibit(acceptor_no, false);
            // WriteAcceptorDetails(acceptor_no, operator&());
        }

        /* 0 -> false, acceptor not detected */
        return (init_ok = static_cast<bool>(acceptor_no));
    }

    void acceptor::update()
    {
    }
}
