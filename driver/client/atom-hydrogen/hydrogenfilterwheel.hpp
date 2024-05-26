/*
 * hydrogenfilterwheel.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Filterwheel

**************************************************/

#ifndef ATOM_HYDROGEN_FILTERWHEEL_HPP
#define ATOM_HYDROGEN_FILTERWHEEL_HPP

#include "atom/driver/filterwheel.hpp"
#include "atom/utils/switch.hpp"
#include "hydrogenbasic.hpp"

class HydrogenFilterwheel : public Filterwheel, public HYDROGEN::BaseClient {
public:
    explicit HydrogenFilterwheel(const std::string &name);
    ~HydrogenFilterwheel();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    virtual bool moveTo(const json &params) override;

    virtual bool getCurrentPosition(const json &params) override;

protected:
    void newDevice(HYDROGEN::BaseDevice dp) override;
    void removeDevice(HYDROGEN::BaseDevice dp) override;
    void newProperty(HYDROGEN::Property property) override;
    void updateProperty(HYDROGEN::Property property) override;
    void removeProperty(HYDROGEN::Property property) override {}
    void newMessage(HYDROGEN::BaseDevice dp, int messageID) override;
    void serverConnected() override;
    void serverDisconnected(int exit_code) override;

    void newSwitch(HYDROGEN::PropertyViewSwitch *svp);
    void newNumber(HYDROGEN::PropertyViewNumber *nvp);
    void newText(HYDROGEN::PropertyViewText *tvp);
    void newBLOB(HYDROGEN::PropertyViewBlob *bp);

protected:
    void ClearStatus();

    // Hydrogen Parameters
private:
    std::shared_ptr<ISwitchVectorProperty> m_connection_prop;
    std::shared_ptr<INumberVectorProperty> filterinfo_prop;
    std::shared_ptr<ITextVectorProperty> filter_port;
    std::shared_ptr<ISwitchVectorProperty> rate_prop;
    std::shared_ptr<ITextVectorProperty> filter_prop;
    HYDROGEN::BaseDevice filter_device;

    std::atomic_bool is_ready;  // 是否就绪
    std::atomic_bool has_blob;  // 是否有 BLOB 数据
    std::atomic_bool is_debug;
    std::atomic_bool is_connected;

    std::string hydrogen_filter_port = "";
    std::string hydrogen_filter_rate = "";

    std::string hydrogen_filter_cmd;
    std::string hydrogen_filter_exec = "";
    std::string hydrogen_filter_version = "";
    std::string hydrogen_filter_interface = "";

    std::unique_ptr<atom::utils::StringSwitch<HYDROGEN::PropertyViewNumber *>>
        m_number_switch;
    std::unique_ptr<atom::utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>>
        m_switch_switch;
    std::unique_ptr<atom::utils::StringSwitch<HYDROGEN::PropertyViewText *>>
        m_text_switch;
};

#endif
