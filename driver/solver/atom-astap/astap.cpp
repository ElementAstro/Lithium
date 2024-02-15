#include "astap.hpp"

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/cmdline.hpp"
#include "atom/system/command.hpp"
#include "atom/async/async.hpp"

#include <fitsio.h>

AstapSolver::AstapSolver(const std::string &name) : Solver(name)
{
    DLOG_F(INFO, "Initializing Astap Solver...");
}

AstapSolver::~AstapSolver()
{
    DLOG_F(INFO, "Destroying Astap Solver...");
}

bool AstapSolver::connect(const json &params)
{
    DLOG_F(INFO, "Connecting to Astap Solver...");
    if (!params.contains("path") || !params["path"].is_string())
    {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    std::string solverPath = params["path"].get<std::string>();
    if (!Atom::IO::isFileNameValid(solverPath) || !Atom::IO::isFileExists(solverPath))
    {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    SetVariable("solverPath", params["path"].get<std::string>());
    DLOG_F(INFO, "Connected to Astap Solver");
    return true;
}

bool AstapSolver::disconnect(const json &params)
{
    DLOG_F(INFO, "Disconnecting from Astap Solver...");
    SetVariable("solverPath", "");
    DLOG_F(INFO, "Disconnected from Astap Solver");
    return true;
}

bool AstapSolver::reconnect(const json &params)
{
    DLOG_F(INFO, "Reconnecting to Astap Solver...");
    if (!disconnect(params))
    {
        return false;
    }
    if (!connect(params))
    {
        return false;
    }
    DLOG_F(INFO, "Reconnected to Astap Solver");
    return true;
}

bool AstapSolver::isConnected()
{
    return GetVariable<std::string>("solverPath").has_value();
}

bool AstapSolver::solveImage(const std::string &image, const int &timeout, const bool &debug)
{
    DLOG_F(INFO, "Solving Image {}...", image);
    if (!isConnected())
    {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", __func__);
        return false;
    }
    if (!Atom::IO::isFileNameValid(image) || !Atom::IO::isFileExists(image))
    {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    SolveResult resu;
    t;
    try
    {
        std::string command = makeCommand();

        auto ret = Atom::Async::asyncRetry([](const std::string &cmd) -> std::string
                              { return Atom::Utils::executeCommand(cmd, false); },
                              3, std::chrono::seconds(5), cmd);

        // 等待命令执行完成，或者超时
        auto start_time = std::chrono::system_clock::now();
        while (ret.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
        {
            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count();
            if (elapsed_time > timeout)
            {
                LOG_F(ERROR, "Error: command timed out after {} seconds.", std::to_string(timeout));
                return "";
            }
        }

        // 返回命令执行结果，并输出调试信息
        auto output = ret.get();
        DLOG_F(INFO, "Command '{}' returned: {}", cmd, output);

        if (output.find("Solution found:") != std::string::npos)
        {
            DLOG_F(INFO, "Solved successfully");
            ret_struct = readSolveResult(image);
        }
        else
        {
            LOG_F(ERROR, "Failed to solve the image");
            ret_struct.error = "Failed to solve the image";
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to execute {}: {}", __func__, e.what());
        return false;
    }
    // 将解析结果写入JSON对象
    if (!result.ra.empty())
    {
        SetVariable("result.ra", result.ra);
    }
    if (!result.dec.empty())
    {
        SetVariable("result.dec", result.dec);
    }
    if (result.fov_x > 0)
    {
        SetVariable("result.fov_x", result.fov_x);
    }
    if (result.fov_y > 0)
    {
        SetVariable("result.fov_y", result.fov_y);
    }
    if (result.fov_avg > 0)
    {
        SetVariable("result.fov_avg", result.fov_avg);
    }
    if (result.rotation != 0)
    {
        SetVariable("result.rotation", result.rotation);
    }
    return true;
}

bool AstapSolver::getSolveResult(const int &timeout, const bool &debug)
{
    DLOG_F(INFO, "Getting Solve Result...");
    return true;
}

bool AstapSolver::getSolveStatus(const int &timeout, const bool &debug)
{
    DLOG_F(INFO, "Getting Solve Status...");
    return true;
}

bool AstapSolver::setSolveParams(const json &params)
{
    DLOG_F(INFO, "Setting Solve Parameters...");
    bool status = true;
    if (params.contains("ra") && params["ra"].is_string())
    {
        DLOG_F(INFO, "Setting Target RA {}", params["ra"].get<std::string>());
        status = SetVariable("target_ra", params["ra"].get<std::string>());
    }
    if (params.contains("dec") && params["dec"].is_string())
    {
        DLOG_F(INFO, "Setting Target Dec {}", params["dec"].get<std::string>());
        status = SetVariable("target_dec", params["dec"].get<std::string>());
    }
    if (params.contains("fov") && params["fov"].is_number())
    {
        DLOG_F(INFO, "Setting Field of View {}", params["fov"].get<double>());
        status = SetVariable("fov", params["fov"].get<double>());
    }
    if (params.contains("update") && params["update"].is_boolean())
    {
        DLOG_F(INFO, "Setting Update {}", params["update"].get<bool>());
        status = SetVariable("update", params["update"].get<bool>());
    }
    return status;
}

json AstapSolver::getSolveParams()
{
    return json{};
}

std::string AstapSolver::makeCommand()
{
    auto solverPath = GetVariable<std::string>("solverPath").value();
    auto image = GetVariable<std::string>("imagePath").value();
    auto ra = GetVariable<std::string>("target_ra").value();
    auto dec = GetVariable<std::string>("target_dec").value();
    auto fov = GetVariable<double>("fov").value();
    auto update = GetVariable<bool>("update").value();

    // Max: Here we should use cmdline.hpp to make the command

    std::stringstream ss;
    ss << solverPath << " -f " << image << "";
    if (!ra.empty())
    {
        ss << " -ra " << ra << "";
    }
    if (!dec.empty())
    {
        ss << " -dec " << dec << "";
    }
    if (fov > 0)
    {
        ss << " -fov " << fov;
    }
    if (update)
    {
        ss << " -update";
    }

    std::string cmd = ss.str();
    DLOG_F(INFO, "Command: {}", cmd);
    return cmd;
}

SolveResult AstapSolver::readSolveResult(const std::string &image)
{
    SovleResult ret_struct;

    // 打开 FITS 文件并读取头信息
    fitsfile *fptr;
    int status = 0;
    fits_open_file(&fptr, image.c_str(), READONLY, &status);
    if (status != 0)
    {
        LOG_F(ERROR, "Failed to read FITS header: {}", image);
        ret_struct.error = "Failed to read FITS header: " + image;
        return ret_struct;
    }

    double solved_ra, solved_dec, x_pixel_arcsec, y_pixel_arcsec, rotation, x_pixel_size, y_pixel_size;
    bool data_get_flag = false;
    char comment[FLEN_COMMENT];

    // 读取头信息中的关键字
    status = 0;
    fits_read_key(fptr, TDOUBLE, "CRVAL1", &solved_ra, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "CRVAL2", &solved_dec, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "CDELT1", &x_pixel_arcsec, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "CDELT2", &y_pixel_arcsec, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "CROTA1", &rotation, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "XPIXSZ", &x_pixel_size, comment, &status);

    status = 0;
    fits_read_key(fptr, TDOUBLE, "YPIXSZ", &y_pixel_size, comment, &status);

    // 关闭 FITS 文件
    fits_close_file(fptr, &status);
    if (status != 0)
    {
        LOG_F(ERROR, "Failed to close FITS file: {}", image);
        ret_struct.error = "Failed to close FITS file: " + image;
        return ret_struct;
    }

    // 构造返回结果
    if (data_get_flag)
    {
        ret_struct.ra = std::to_string(solved_ra);
        ret_struct.dec = std::to_string(solved_dec);
        ret_struct.rotation = std::to_string(rotation);
        if (x_pixel_size > 0.0 && y_pixel_size > 0.0)
        {
            double x_focal_length = x_pixel_size / x_pixel_arcsec * 206.625;
            double y_focal_length = y_pixel_size / y_pixel_arcsec * 206.625;
            double avg_focal_length = (x_focal_length + y_focal_length) / 2.0;
            ret_struct.fov_x = x_focal_length;
            ret_struct.fov_y = y_focal_length;
            ret_struct.fov_avg = avg_focal_length;
            // 调试输出
            DLOG_F(INFO, "avg_focal_length: {}", avg_focal_length);
        }
    }
    else
    {
        LOG_F(ERROR, "Solve failed");
        ret_struct.error = "Solve failed";
    }
    return ret_struct;
}