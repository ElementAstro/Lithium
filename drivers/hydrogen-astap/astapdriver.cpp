/*******************************************************************************
  Copyright(c) 2017 Jasem Mutlaq. All rights reserved.

  HYDROGEN Astap.net Driver

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#include "astapdriver.h"

#include <memory>

#include <cerrno>
#include <cstring>

#include <zlib.h>

// We declare an auto pointer to AstapDriver.
std::unique_ptr<AstapDriver> astap = std::make_unique<AstapDriver>();

AstapDriver::AstapDriver()
{
    setVersion(1, 0);
}

bool AstapDriver::initProperties()
{
    HYDROGEN::DefaultDevice::initProperties();

    /**********************************************/
    /**************** Astap ******************/
    /**********************************************/

    // Solver Enable/Disable
    IUFillSwitch(&SolverS[SOLVER_ENABLE], "ASTAP_SOLVER_ENABLE", "Enable", ISS_OFF);
    IUFillSwitch(&SolverS[SOLVER_DISABLE], "ASTAP_SOLVER_DISABLE", "Disable", ISS_ON);
    IUFillSwitchVector(&SolverSP, SolverS, 2, getDeviceName(), "ASTAP_SOLVER", "Solver", MAIN_CONTROL_TAB, IP_RW,
                       ISR_1OFMANY, 0, IPS_IDLE);

    // Solver Settings
    IUFillText(&SolverSettingsT[ASTAP_SETTINGS_BINARY], "ASTAP_SETTINGS_BINARY", "Solver",
               "/usr/bin/solve-field");
    IUFillText(&SolverSettingsT[ASTAP_SETTINGS_OPTIONS], "ASTAP_SETTINGS_OPTIONS", "Options",
               "--no-verify --no-plots --resort --downsample 2 -O");
    IUFillTextVector(&SolverSettingsTP, SolverSettingsT, 2, getDeviceName(), "ASTAP_SETTINGS", "Settings",
                     MAIN_CONTROL_TAB, IP_WO, 0, IPS_IDLE);

    // Solver Results
    IUFillNumber(&SolverResultN[ASTAP_RESULTS_PIXSCALE], "ASTAP_RESULTS_PIXSCALE", "Pixscale (arcsec/pixel)",
                 "%g", 0, 10000, 1, 0);
    IUFillNumber(&SolverResultN[ASTAP_RESULTS_ORIENTATION], "ASTAP_RESULTS_ORIENTATION",
                 "Orientation (E of N) °", "%g", -360, 360, 1, 0);
    IUFillNumber(&SolverResultN[ASTAP_RESULTS_RA], "ASTAP_RESULTS_RA", "RA (J2000)", "%g", 0, 24, 1, 0);
    IUFillNumber(&SolverResultN[ASTAP_RESULTS_DE], "ASTAP_RESULTS_DE", "DE (J2000)", "%g", -90, 90, 1, 0);
    IUFillNumber(&SolverResultN[ASTAP_RESULTS_PARITY], "ASTAP_RESULTS_PARITY", "Parity", "%g", -1, 1, 1, 0);
    IUFillNumberVector(&SolverResultNP, SolverResultN, 5, getDeviceName(), "ASTAP_RESULTS", "Results",
                       MAIN_CONTROL_TAB, IP_RO, 0, IPS_IDLE);

    // Solver Data Blob
    IUFillBLOB(&SolverDataB[0], "ASTAP_DATA_BLOB", "Image", "");
    IUFillBLOBVector(&SolverDataBP, SolverDataB, 1, getDeviceName(), "ASTAP_DATA", "Upload", MAIN_CONTROL_TAB,
                     IP_WO, 60, IPS_IDLE);

    /**********************************************/
    /**************** Snooping ********************/
    /**********************************************/

    // Snooped Devices
    IUFillText(&ActiveDeviceT[0], "ACTIVE_CCD", "CCD", "CCD Simulator");
    IUFillTextVector(&ActiveDeviceTP, ActiveDeviceT, 1, getDeviceName(), "ACTIVE_DEVICES", "Snoop devices", OPTIONS_TAB,
                     IP_RW, 60, IPS_IDLE);

    // Primary CCD Chip Data Blob
    IUFillBLOB(&CCDDataB[0], "CCD1", "Image", "");
    IUFillBLOBVector(&CCDDataBP, CCDDataB, 1, ActiveDeviceT[0].text, "CCD1", "Image Data", "Image Info", IP_RO, 60,
                     IPS_IDLE);

    IDSnoopDevice(ActiveDeviceT[0].text, "CCD1");
    IDSnoopBLOBs(ActiveDeviceT[0].text, "CCD1", B_ONLY);

    addDebugControl();

    setDriverInterface(AUX_INTERFACE);

    return true;
}

void AstapDriver::ISGetProperties(const char *dev)
{
    DefaultDevice::ISGetProperties(dev);

    defineProperty(&ActiveDeviceTP);
    loadConfig(true, "ACTIVE_DEVICES");
}

bool AstapDriver::updateProperties()
{
    HYDROGEN::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineProperty(&SolverSP);
        defineProperty(&SolverSettingsTP);
        defineProperty(&SolverDataBP);
    }
    else
    {
        if (SolverS[0].s == ISS_ON)
        {
            deleteProperty(SolverResultNP.name);
        }
        deleteProperty(SolverSP.name);
        deleteProperty(SolverSettingsTP.name);
        deleteProperty(SolverDataBP.name);
    }

    return true;
}

const char *AstapDriver::getDefaultName()
{
    return "Astap";
}

bool AstapDriver::Connect()
{
    return true;
}

bool AstapDriver::Disconnect()
{
    return true;
}

bool AstapDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    return HYDROGEN::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool AstapDriver::ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
                                 char *formats[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, SolverDataBP.name) == 0)
        {
            SolverDataBP.s = IPS_OK;
            IDSetBLOB(&SolverDataBP, nullptr);

            // If the client explicitly uploaded the data then we solve it.
            if (SolverS[SOLVER_ENABLE].s == ISS_OFF)
            {
                SolverS[SOLVER_ENABLE].s = ISS_ON;
                SolverS[SOLVER_DISABLE].s = ISS_OFF;
                SolverSP.s   = IPS_BUSY;
                LOG_INFO("Astap solver is enabled.");
                defineProperty(&SolverResultNP);
            }

            processBLOB(reinterpret_cast<uint8_t *>(blobs[0]), static_cast<uint32_t>(sizes[0]),
                        static_cast<uint32_t>(blobsizes[0]));

            return true;
        }
    }

    return HYDROGEN::DefaultDevice::ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

bool AstapDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        //  This is for our device
        //  Now lets see if it's something we process here
        if (strcmp(name, ActiveDeviceTP.name) == 0)
        {
            ActiveDeviceTP.s = IPS_OK;
            IUUpdateText(&ActiveDeviceTP, texts, names, n);
            IDSetText(&ActiveDeviceTP, nullptr);

            // Update the property name!
            strncpy(CCDDataBP.device, ActiveDeviceT[0].text, MAXHYDROGENDEVICE);
            IDSnoopDevice(ActiveDeviceT[0].text, "CCD1");
            IDSnoopBLOBs(ActiveDeviceT[0].text, "CCD1", B_ONLY);

            //  We processed this one, so, tell the world we did it
            return true;
        }

        if (strcmp(name, SolverSettingsTP.name) == 0)
        {
            IUUpdateText(&SolverSettingsTP, texts, names, n);
            SolverSettingsTP.s = IPS_OK;
            IDSetText(&SolverSettingsTP, nullptr);
            return true;
        }
    }

    return HYDROGEN::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool AstapDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Astap Enable/Disable
        if (strcmp(name, SolverSP.name) == 0)
        {
            pthread_mutex_lock(&lock);

            IUUpdateSwitch(&SolverSP, states, names, n);
            SolverSP.s = IPS_OK;

            if (SolverS[SOLVER_ENABLE].s == ISS_ON)
            {
                LOG_INFO("Astap solver is enabled.");
                defineProperty(&SolverResultNP);
            }
            else
            {
                LOG_INFO("Astap solver is disabled.");
                deleteProperty(SolverResultNP.name);
            }

            IDSetSwitch(&SolverSP, nullptr);

            pthread_mutex_unlock(&lock);
            return true;
        }
    }

    return HYDROGEN::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool AstapDriver::ISSnoopDevice(XMLEle *root)
{
    if (SolverS[SOLVER_ENABLE].s == ISS_ON && IUSnoopBLOB(root, &CCDDataBP) == 0)
    {
        processBLOB(reinterpret_cast<uint8_t *>(CCDDataB[0].blob), static_cast<uint32_t>(CCDDataB[0].size),
                    static_cast<uint32_t>(CCDDataB[0].bloblen));
        return true;
    }

    return HYDROGEN::DefaultDevice::ISSnoopDevice(root);
}

bool AstapDriver::saveConfigItems(FILE *fp)
{
    IUSaveConfigText(fp, &ActiveDeviceTP);
    IUSaveConfigText(fp, &SolverSettingsTP);
    return true;
}

bool AstapDriver::processBLOB(uint8_t *data, uint32_t size, uint32_t len)
{
    FILE *fp = nullptr;
    char imageFileName[MAXRBUF];

    uint8_t *processedData = data;

    // If size != len then we have compressed buffer
    if (size != len)
    {
        uint8_t *dataBuffer = new uint8_t[size];
        uLongf destLen      = size;

        if (dataBuffer == nullptr)
        {
            LOG_DEBUG("Unable to allocate memory for data buffer");
            return false;
        }

        int r = uncompress(dataBuffer, &destLen, data, len);
        if (r != Z_OK)
        {
            LOGF_ERROR("Astap compression error: %d", r);
            delete[] dataBuffer;
            return false;
        }

        if (destLen != size)
        {
            LOGF_WARN("Discrepency between uncompressed data size %ld and expected size %ld",
                      size, destLen);
        }

        processedData = dataBuffer;
    }

    strncpy(imageFileName, "/tmp/ccdsolver.fits", MAXRBUF);

    fp = fopen(imageFileName, "w");
    if (fp == nullptr)
    {
        LOGF_ERROR("Unable to save image file (%s). %s", imageFileName, strerror(errno));
        if (size != len)
            delete[] processedData;

        return false;
    }

    int n = 0;
    for (uint32_t nr = 0; nr < size; nr += n)
        n = fwrite(processedData + nr, 1, size - nr, fp);

    fclose(fp);

    // Do not forget to release uncompressed buffer
    if (size != len)
        delete[] processedData;

    pthread_mutex_lock(&lock);
    SolverSP.s = IPS_BUSY;
    LOG_INFO("Solving image...");
    IDSetSwitch(&SolverSP, nullptr);
    pthread_mutex_unlock(&lock);

    int result = pthread_create(&solverThread, nullptr, &AstapDriver::runSolverHelper, this);

    if (result != 0)
    {
        SolverSP.s = IPS_ALERT;
        LOGF_INFO("Failed to create solver thread: %s", strerror(errno));
        IDSetSwitch(&SolverSP, nullptr);
    }

    return true;
}

void *AstapDriver::runSolverHelper(void *context)
{
    (static_cast<AstapDriver *>(context))->runSolver();
    return nullptr;
}

void AstapDriver::runSolver()
{
    char cmd[MAXRBUF] = {0}, line[256] = {0}, parity_str[8] = {0};
    float ra = -1000, dec = -1000, angle = -1000, pixscale = -1000, parity = 0;
    snprintf(cmd, MAXRBUF, "%s %s -W /tmp/solution.wcs /tmp/ccdsolver.fits",
             SolverSettingsT[ASTAP_SETTINGS_BINARY].text, SolverSettingsT[ASTAP_SETTINGS_OPTIONS].text);

    LOGF_DEBUG("%s", cmd);
    FILE *handle = popen(cmd, "r");
    if (handle == nullptr)
    {
        LOGF_DEBUG("Failed to run solver: %s", strerror(errno));
        pthread_mutex_lock(&lock);
        SolverSP.s = IPS_ALERT;
        IDSetSwitch(&SolverSP, nullptr);
        pthread_mutex_unlock(&lock);
        return;
    }

    while (fgets(line, sizeof(line), handle) != nullptr)
    {
        LOGF_DEBUG("%s", line);

        sscanf(line, "Field rotation angle: up is %f", &angle);
        sscanf(line, "Field center: (RA,Dec) = (%f,%f)", &ra, &dec);
        sscanf(line, "Field parity: %s", parity_str);
        sscanf(line, "%*[^p]pixel scale %f", &pixscale);

        if (strcmp(parity_str, "pos") == 0)
            parity = 1;
        else if (strcmp(parity_str, "neg") == 0)
            parity = -1;

        if (ra != -1000 && dec != -1000 && angle != -1000 && pixscale != -1000)
        {
            // Pixscale is arcsec/pixel. Astap result is in arcmin
            SolverResultN[ASTAP_RESULTS_PIXSCALE].value = pixscale;
            // Astap.net angle, E of N
            SolverResultN[ASTAP_RESULTS_ORIENTATION].value = angle;
            // Astap.net J2000 RA in degrees
            SolverResultN[ASTAP_RESULTS_RA].value = ra;
            // Astap.net J2000 DEC in degrees
            SolverResultN[ASTAP_RESULTS_DE].value = dec;
            // Astap.net parity
            SolverResultN[ASTAP_RESULTS_PARITY].value = parity;

            SolverResultNP.s = IPS_OK;
            IDSetNumber(&SolverResultNP, nullptr);

            pthread_mutex_lock(&lock);
            SolverSP.s = IPS_OK;
            IDSetSwitch(&SolverSP, nullptr);
            pthread_mutex_unlock(&lock);

            pclose(handle);
            LOG_INFO("Solver complete.");
            return;
        }

        pthread_mutex_lock(&lock);
        if (SolverS[SOLVER_DISABLE].s == ISS_ON)
        {
            SolverSP.s = IPS_IDLE;
            IDSetSwitch(&SolverSP, nullptr);
            pthread_mutex_unlock(&lock);
            pclose(handle);
            LOG_INFO("Solver canceled.");
            return;
        }
        pthread_mutex_unlock(&lock);
    }

    pclose(handle);

    pthread_mutex_lock(&lock);
    SolverSP.s = IPS_ALERT;
    IDSetSwitch(&SolverSP, nullptr);
    LOG_INFO("Solver failed.");
    pthread_mutex_unlock(&lock);

    pthread_exit(nullptr);
}
