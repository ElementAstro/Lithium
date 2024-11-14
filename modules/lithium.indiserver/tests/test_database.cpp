#include <gtest/gtest.h>
#include <fstream>
#include "database.hpp"

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary database file
        db_filename = "test_database.json";
        std::ofstream(db_filename) << R"({
            "version": "0.1.6",
            "profiles": [
                {"name": "Profile1", "port": 7624, "autostart": false, "autoconnect": false, "drivers": []},
                {"name": "Profile2", "port": 7625, "autostart": true, "autoconnect": false, "drivers": []}
            ],
            "custom_drivers": [],
            "remote_drivers": []
        })";
        db = std::make_unique<Database>(db_filename);
    }

    void TearDown() override {
        // Remove the temporary database file
        std::filesystem::remove(db_filename);
    }

    std::string db_filename;
    std::unique_ptr<Database> db;
};

TEST_F(DatabaseTest, UpdateExistingProfile) {
    db->updateProfile("Profile1", 7626, true, true);

    auto profile = db->getProfile("Profile1");
    ASSERT_TRUE(profile.has_value());
    EXPECT_EQ(profile->at("port"), 7626);
    EXPECT_TRUE(profile->at("autostart"));
    EXPECT_TRUE(profile->at("autoconnect"));

    auto other_profile = db->getProfile("Profile2");
    ASSERT_TRUE(other_profile.has_value());
    EXPECT_FALSE(other_profile->at("autostart"));
}

TEST_F(DatabaseTest, UpdateNonExistingProfile) {
    db->updateProfile("NonExistingProfile", 7626, true, true);

    auto profile = db->getProfile("NonExistingProfile");
    EXPECT_FALSE(profile.has_value());

    auto existing_profile = db->getProfile("Profile1");
    ASSERT_TRUE(existing_profile.has_value());
    EXPECT_EQ(existing_profile->at("port"), 7624);
    EXPECT_FALSE(existing_profile->at("autostart"));
    EXPECT_FALSE(existing_profile->at("autoconnect"));
}

TEST_F(DatabaseTest, UpdateProfileAutostartAutoconnectCombinations) {
    db->updateProfile("Profile1", 7626, false, true);

    auto profile = db->getProfile("Profile1");
    ASSERT_TRUE(profile.has_value());
    EXPECT_EQ(profile->at("port"), 7626);
    EXPECT_FALSE(profile->at("autostart"));
    EXPECT_TRUE(profile->at("autoconnect"));

    db->updateProfile("Profile1", 7627, true, false);

    profile = db->getProfile("Profile1");
    ASSERT_TRUE(profile.has_value());
    EXPECT_EQ(profile->at("port"), 7627);
    EXPECT_TRUE(profile->at("autostart"));
    EXPECT_FALSE(profile->at("autoconnect"));

    auto other_profile = db->getProfile("Profile2");
    ASSERT_TRUE(other_profile.has_value());
    EXPECT_FALSE(other_profile->at("autostart"));
}