#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <libstellarsolver/stellarsolver.h>
#include <libstellarsolver/structuredefinitions.h>

#include <QCoreApplication>
#include <QObject>
#include <QPointF>
#include <QRect>
#include <QStringList>

namespace py = pybind11;

class SS : public QObject {
    Q_OBJECT
public:
    explicit SS(QObject* parent = nullptr);
    SS(const FITSImage::Statistic& stat, py::buffer buffer, py::object callback,
       QObject* parent = nullptr);
    ~SS();

    // StellarSolver 方法
    bool loadNewImageBuffer(const FITSImage::Statistic& stats,
                            py::buffer buffer);

    static SSolver::ExternalProgramPaths getDefaultExternalPaths(
        SSolver::ComputerSystemType system);
    static SSolver::ExternalProgramPaths getDefaultExternalPaths();

    static QStringList getIndexFiles(const QStringList& directoryList,
                                     int indexToUse = -1,
                                     int healpixToUse = -1);

    bool extract(bool calculateHFR = false, QRect frame = QRect());
    bool solve();
    void start();
    void abort();
    void abortAndWait();

    void setParameterProfile(SSolver::Parameters::ParametersProfile profile);

    void setSearchScale(double fovLow, double fovHigh,
                        const QString& scaleUnits);
    void setSearchScale(double fovLow, double fovHigh,
                        SSolver::ScaleUnits units);

    void setSearchPositionRaDec(double ra, double dec);
    void setSearchPositionInDegrees(double ra, double dec);

    static QVector<float> generateConvFilter(SSolver::ConvFilterType filter,
                                             double fwhm);
    static QList<SSolver::Parameters> getBuiltInProfiles();
    static QList<SSolver::Parameters> loadSavedOptionsProfiles(
        const QString& savedOptionsProfiles);
    static QStringList getDefaultIndexFolderPaths();

    void setUseSubframe(QRect frame);
    bool isRunning() const;

    static QString raString(double ra);
    static QString decString(double dec);

    bool pixelToWCS(const QPointF& pixelPoint, FITSImage::wcs_point& skyPoint);
    bool wcsToPixel(const FITSImage::wcs_point& skyPoint, QPointF& pixelPoint);

signals:
    void logOutput(const QString& logText);
    void ready();
    void finished();

private slots:
    void onLogOutput(const QString& text);
    void onFinished();

private:
    py::dict createObjectFromStar(const FITSImage::Star& star);

    // 成员变量
    QCoreApplication* app;
    StellarSolver* solver;
    py::object callback_;
    std::vector<uint8_t> bufferData_;
};