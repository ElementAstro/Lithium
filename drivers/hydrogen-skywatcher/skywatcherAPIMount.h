/*!
 * \file skywatcherAPIMount.h
 *
 * \author Roger James
 * \author Gerry Rozema
 * \author Jean-Luc Geehalel
 * \date 13th November 2013
 *
 * This file contains the definitions for a C++ implementatiom of a HYDROGEN telescope driver using the Skywatcher API.
 * It is based on work from three sources.
 * A C++ implementation of the API by Roger James.
 * The hydrogen_eqmod driver by Jean-Luc Geehalel.
 * The synscanmount driver by Gerry Rozema.
 */

#pragma once

#include "hydrogenguiderinterface.h"
#include "skywatcherAPI.h"
#include "hydrogenelapsedtimer.h"
#include "hydrogenpropertynumber.h"
#include "hydrogenpropertyswitch.h"
#include "alignment/AlignmentSubsystemForDrivers.h"
#include "pid/pid.h"
#include <numeric>

typedef enum { PARK_COUNTERCLOCKWISE = 0, PARK_CLOCKWISE } ParkDirection_t;
typedef enum { PARK_NORTH = 0, PARK_EAST, PARK_SOUTH, PARK_WEST } ParkPosition_t;

struct GuidingPulse
{
    double DeltaAlt { 0 };
    double DeltaAz { 0 };
    int Duration { 0 };
    int OriginalDuration { 0 };
};


class SkywatcherAPIMount :
    public SkywatcherAPI,
    public HYDROGEN::Telescope,
    public HYDROGEN::GuiderInterface,
    public HYDROGEN::AlignmentSubsystem::AlignmentSubsystemForDrivers
{
    public:
        SkywatcherAPIMount();
        virtual ~SkywatcherAPIMount() override = default;

        virtual bool initProperties() override;
        virtual void ISGetProperties(const char *dev) override;
        virtual bool updateProperties() override;
        virtual bool ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
                               char *formats[], char *names[], int n) override;
        virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;

    protected:
        /////////////////////////////////////////////////////////////////////////////////////
        /// Communication
        /////////////////////////////////////////////////////////////////////////////////////
        virtual bool Handshake() override;
        virtual bool ReadScopeStatus() override;

        /////////////////////////////////////////////////////////////////////////////////////
        /// Motion
        /////////////////////////////////////////////////////////////////////////////////////
        virtual bool MoveNS(HYDROGEN_DIR_NS dir, TelescopeMotionCommand command) override;
        virtual bool MoveWE(HYDROGEN_DIR_WE dir, TelescopeMotionCommand command) override;
        virtual bool Goto(double ra, double dec) override;
        virtual bool Sync(double ra, double dec) override;
        virtual bool Abort() override;
        virtual bool SetTrackEnabled(bool enabled) override;

        /////////////////////////////////////////////////////////////////////////////////////
        /// Misc.
        /////////////////////////////////////////////////////////////////////////////////////
        virtual const char *getDefaultName() override;
        virtual void TimerHit() override;
        virtual bool updateLocation(double latitude, double longitude, double elevation) override;
        virtual bool saveConfigItems(FILE *fp) override;
        double GetSlewRate();
        double GetParkDeltaAz(ParkDirection_t target_direction, ParkPosition_t target_position);

        /////////////////////////////////////////////////////////////////////////////////////
        /// Guiding
        /////////////////////////////////////////////////////////////////////////////////////
        virtual IPState GuideNorth(uint32_t ms) override;
        virtual IPState GuideSouth(uint32_t ms) override;
        virtual IPState GuideEast(uint32_t ms) override;
        virtual IPState GuideWest(uint32_t ms) override;

        /////////////////////////////////////////////////////////////////////////////////////
        /// Parking
        /////////////////////////////////////////////////////////////////////////////////////
        virtual bool Park() override;
        virtual bool UnPark() override;
        virtual bool SetCurrentPark() override;
        virtual bool SetDefaultPark() override;

    private:

        /////////////////////////////////////////////////////////////////////////////////////
        /// Guiding
        /////////////////////////////////////////////////////////////////////////////////////
        void CalculateGuidePulses();
        void ResetGuidePulses();
        void ConvertGuideCorrection(double delta_ra, double delta_dec, double &delta_alt, double &delta_az);

        /////////////////////////////////////////////////////////////////////////////////////
        /// Telescope Vector <---> Microsteps
        /////////////////////////////////////////////////////////////////////////////////////
        void SkywatcherMicrostepsFromTelescopeDirectionVector(
            const HYDROGEN::AlignmentSubsystem::TelescopeDirectionVector TelescopeDirectionVector, long &Axis1Microsteps,
            long &Axis2Microsteps);
        const HYDROGEN::AlignmentSubsystem::TelescopeDirectionVector
        TelescopeDirectionVectorFromSkywatcherMicrosteps(long Axis1Microsteps, long Axis2Microsteps);

        /////////////////////////////////////////////////////////////////////////////////////
        /// Misc
        /////////////////////////////////////////////////////////////////////////////////////
        void UpdateDetailedMountInformation(bool InformClient);
        bool getCurrentAltAz(HYDROGEN::IHorizontalCoordinates &altaz);
        bool getCurrentRADE(HYDROGEN::IHorizontalCoordinates altaz, HYDROGEN::IEquatorialCoordinates &rade);
        // Reset tracking timer to account for drift compensation
        void resetTracking();
        inline double average(const std::vector<double> &values)
        {
            return values.empty() ? 0 : std::accumulate(values.begin(), values.end(), 0.0) / values.size();
        }

        /////////////////////////////////////////////////////////////////////////////////////
        /// Properties
        /////////////////////////////////////////////////////////////////////////////////////
        static constexpr const char *MountInfoTab { "Mount Info" };
        IText BasicMountInfoT[4] {};
        ITextVectorProperty BasicMountInfoTP;
        enum
        {
            MOTOR_CONTROL_FIRMWARE_VERSION,
            MOUNT_CODE,
            MOUNT_NAME,
            IS_DC_MOTOR
        };

        INumber AxisOneInfoN[4];
        INumberVectorProperty AxisOneInfoNP;
        INumber AxisTwoInfoN[4];
        INumberVectorProperty AxisTwoInfoNP;
        enum
        {
            MICROSTEPS_PER_REVOLUTION,
            STEPPER_CLOCK_FREQUENCY,
            HIGH_SPEED_RATIO,
            MICROSTEPS_PER_WORM_REVOLUTION
        };


        ISwitch AxisOneStateS[6];
        ISwitchVectorProperty AxisOneStateSP;
        ISwitch AxisTwoStateS[6];
        ISwitchVectorProperty AxisTwoStateSP;
        enum
        {
            FULL_STOP,
            SLEWING,
            SLEWING_TO,
            SLEWING_FORWARD,
            HIGH_SPEED,
            NOT_INITIALISED
        };

        enum
        {
            RAW_MICROSTEPS,
            MICROSTEPS_PER_ARCSEC,
            OFFSET_FROM_INITIAL,
            DEGREES_FROM_INITIAL
        };
        INumber AxisOneEncoderValuesN[4];
        INumberVectorProperty AxisOneEncoderValuesNP;
        INumber AxisTwoEncoderValuesN[4];
        INumberVectorProperty AxisTwoEncoderValuesNP;

        ISwitch SlewModesS[2];
        ISwitchVectorProperty SlewModesSP;
        // A switch for silent/highspeed slewing modes
        enum
        {
            SLEW_SILENT,
            SLEW_NORMAL
        };

        ISwitch SoftPECModesS[2];
        ISwitchVectorProperty SoftPECModesSP;
        // A switch for SoftPEC modes
        enum
        {
            SOFTPEC_ENABLED,
            SOFTPEC_DISABLED
        };

        // SoftPEC value for tracking mode
        INumber SoftPecN;
        INumberVectorProperty SoftPecNP;

        // Guiding rates (RA/Dec)
        INumber GuidingRatesN[2];
        INumberVectorProperty GuidingRatesNP;

        // PID controllers
        HYDROGEN::PropertyNumber Axis1PIDNP {3};
        HYDROGEN::PropertyNumber Axis2PIDNP {3};
        enum
        {
            Propotional,
            Derivative,
            Integral
        };

        // Dead Zone
        HYDROGEN::PropertyNumber AxisDeadZoneNP {2};

        // Clock Rate Multiplier
        HYDROGEN::PropertyNumber AxisClockNP {2};

        // Offset
        HYDROGEN::PropertyNumber AxisOffsetNP {5};
        enum
        {
            RAOffset,
            DEOffset,
            AZSteps,
            ALSteps,
            JulianOffset,
         };

         // Axis 1 Direct Track Control
         HYDROGEN::PropertyNumber Axis1TrackRateNP {2};
         HYDROGEN::PropertyNumber Axis2TrackRateNP {2};
         enum
         {
            TrackDirection,
            TrackClockRate,
         };

        // AUX Encoders
        HYDROGEN::PropertySwitch AUXEncoderSP {2};

        // Snap Port
        HYDROGEN::PropertySwitch SnapPortSP {2};

        /////////////////////////////////////////////////////////////////////////////////////
        /// Private Variables
        /////////////////////////////////////////////////////////////////////////////////////
        // Tracking
        HYDROGEN::IEquatorialCoordinates m_SkyTrackingTarget { 0, 0 };
        HYDROGEN::IEquatorialCoordinates m_SkyCurrentRADE {0, 0};
        HYDROGEN::IHorizontalCoordinates m_MountAltAz {0, 0};

        std::unique_ptr<PID> m_Controllers[2];

        // Maximum delta to track. If drift is above 5 degrees, we abort tracking.
        static constexpr double MAX_TRACKING_DELTA {5};    
        static constexpr const char *TRACKING_TAB = "Tracking";

        HYDROGEN::ElapsedTimer m_TrackingRateTimer;
        uint8_t m_LastCustomDirection[2];
        double GuideDeltaAlt { 0 };
        double GuideDeltaAz { 0 };

        GuidingPulse NorthPulse;
        GuidingPulse WestPulse;
        std::vector<GuidingPulse> GuidingPulses;

        bool m_ManualMotionActive { false };
        bool m_IterativeGOTOPending {false};
};