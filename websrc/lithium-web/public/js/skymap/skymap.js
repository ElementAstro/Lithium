function initAll() {
    getLocalStorageMultiFov(),
        getLocalStorageAlad(),
        getLocalStorageMosaic(),
        initAladin(),
        $(".aladin-location").prepend(
            "<span class='aladin-location-text aladin-reticleColor' id='specCoord'>J2000 </span>"
        ),
        createAlertFov(),
        checkFovIncompleteData(fovdata.DX, fovdata.DY),
        populateDdProfileMenu(),
        $(window).trigger("resize"),
        $(".alert").hide(),
        $(function () {
            $('[data-toggle="tooltip"]').tooltip();
        }),
        firstDelayedRedraw(),
        initPAbuttons();
}
function resetInterfaceOnRun() {
    resetEnvironmentData(),
        resetShotPanel(),
        resetImageGraph(),
        $(".imgStatItem").remove(),
        resetTempGraph(),
        resetGuideGraph(),
        resetGlobalvar(),
        $(".btn").css("pointer-events", "auto"),
        $(".btn").addClass("disabled btn-dark"),
        $(".statusLed").removeClass("ledgreen"),
        $(".blindSwitchInput").prop("checked", !1),
        $("body").css("cursor", "default"),
        $("#startupConn").html("Connect"),
        resetInfoDataItem(),
        stopAnimLogo(),
        hideStatusBar(),
        $("#seqRunli").addClass("d-none").removeClass("d-flex"),
        $("#dsRunli").addClass("d-none").removeClass("d-flex"),
        setInitialBtnActive();
}
function resetInfoDataItem() {
    var e = document.querySelectorAll(".info-data-item:not(.text-muted)");
    e.forEach(function (e) {
        e.textContent = "";
    }),
        (e.length = 0);
}
function resetGlobalvar() {
    (imgShooting = !1),
        (tempUIDshot = ""),
        (tempUIDsearch = ""),
        (startupConnected = !1),
        (voyIsRunning = !1),
        (imageStats = []),
        (cameraStatus = ""),
        (mountConnected = !1),
        (ccdConnected = !1),
        (ccdCoolerOn = !1),
        (planConnected = !1),
        (guideConnected = !1),
        (autofocusConnected = !1),
        (mountTracking = !1),
        (mountParked = !1),
        (mountFlip = 0);
}
function renderAladin() {
    (aladinTarget = ""),
        void 0 !== fovdata.radec
            ? (aladinTarget = fovdata.radec)
            : (aladinTarget = "00 42 44.330, +41 16 7.50"),
        (aladin = A.aladin("#aladin-lite-div", {
            cooFrame: "j2000",
            survey: "P/DSS2/color",
            target: aladinTarget,
            fov: 1,
            showReticle: !0,
            showGotoControl: !1,
            showLayersControl: !1,
            showShareControl: !1,
            showZoomControl: !1,
            showFullscreenControl: !1,
            showSimbadPointerControl: simbadPointer,
            showFrame: !0,
            reticleSize: 25,
            reticleColor: "#ff0000",
            log: !1
        })),
        null == fovdata.pixsize &&
        (populateFovDataValues(multiFov.Virtual_Fov_Default),
            (actualProfileName = "Virtual_Fov_Default"),
            (fovdata.profileName = actualProfileName)),
        $("#search_com").click(function () {
            var e = jQuery("#inputsearch").val();
            window.aladin.gotoObject(e, {
                success: function () {
                    getCoordinateAladin(), setTimeout(reDrawVfov, 1200);
                },
                error: function () {
                    errorFire("Aladin Search: object not found");
                }
            });
        }),
        $("#resetPA").click(function () {
            changePAng(0);
        }),
        document
            .getElementById("fdb-brightness")
            .addEventListener("input", function (e) {
                var t = "brightness(" + this.value + "%)";
                jQuery(".aladin-imageCanvas").css({
                    "-webkit-filter": t,
                    "-moz-filter": t,
                    "-ms-filter": t,
                    "-o-filter": t,
                    filter: t
                });
            }),
        document.getElementById("fdb-pAng").addEventListener("input", function (e) {
            changePAng(this.value);
        }),
        jQuery("#inputsearch").val(""),
        $("#paInd").text(" " + fovdata.pAng + "°"),
        $("#fdb-pAng").val(fovdata.pAng),
        $("#fdb-brightness").val(100),
        getCoordinateAladin(),
        $(".aladin-reticleCanvas").unbind("mousewheel"),
        $("#simulatorSelected").attr("disabled", !0);
    let e = document.querySelector(".aladin-reticleCanvas");
    e.addEventListener("mouseup", reticleMouseUpAladin),
        e.addEventListener(
            "touchend",
            function (t) {
                var o = new MouseEvent("mouseup", {});
                e.dispatchEvent(o);
            },
            !1
        );
    var t = $("#aladin-lite-div").width(),
        o = $("#aladin-lite-div").height();
    createframeFoV(
        fovdata.fovx,
        fovdata.fovy,
        "#ffffff",
        t,
        o,
        fovdata.reso,
        0,
        !0,
        !0,
        "LighAPTFovSimulator"
    );
}
function calcfov(e, t, o, a) {
    // e : int # width of the sensor
    // t : int # height of the sensor
    // o : float # pixel size
    // a : focal length
    var n = 0.016666666666667 * Math.round(3438 * Math.atan(e / a)),
        r = 0.016666666666667 * Math.round(3438 * Math.atan(t / a));
    (n = n.toFixed(2)), (r = r.toFixed(2));
    var i = Math.round(100 * Math.atan(o / a) * 206, 3) / 100,
        s = [n, r, i];
    return s;
    // n : width of fov
    // r : height of fov
    // i : samples
}
function getCoordinateAladin() {
    (raDec = aladin.getRaDec()),
        (fovdata.radec = raDec[0] + ", " + raDec[1]),
        (ra = raDec[0] / 15),
        (dec = raDec[1]),
        $("#fovsimRA").text(coordinateFormat(ra, "h")),
        $("#fovsimDEC").text(coordinateFormat(dec, "º")),
        setClipboardData(raDec),
        saveLocalStorageAlad();
}
function convertRaInSexad(e) {
    var t = (e / 15).toTrunc(),
        o = (4 * (e - 15 * t)).toTrunc(),
        a = 240 * (e - 15 * t - o / 4);
    return t + " " + o + " " + a.toFixed(3);
}
function convertDecInSexad() {
    var e = "",
        t = dec.toTrunc(),
        o = (60 * (dec - t)).toTrunc(),
        a = Math.abs(3600 * (dec - t - o / 60));
    return (
        (o = Math.abs(o)),
        (e = 0 === t && dec < 0 ? "-" : ""),
        e + t + " " + o + " " + a.toFixed(3)
    );
}
function getCoordinateAladinRoboClip() {
    $(".rcField").val("");
    let e = $("#fovsimRA").text(),
        t = $("#fovsimDEC").text(),
        o = fovdata.pAng,
        a = generateUID();
    $("#raRc").val(e),
        $("#decRc").val(t),
        $("#paRc").val(o, 2),
        $("#rcUid").val(a),
        chkIfMos()
            ? $("#ledMosaicRoboclip").text(
                "Mosaic [" + mosaicSettings.Hnum + "x" + mosaicSettings.Wnum + "]"
            )
            : $("#ledMosaicRoboclip").text(""),
        enableSaveDelBtn(!0),
        showRcAlert(
            "#rcSuccess",
            "Coords and PA retreived from actual Virtual FOV"
        );
}
function toFixedTrunc(e, t) {
    const o = ("string" == typeof e ? e : e.toString()).split(".");
    if (t <= 0) return o[0];
    let a = o[1] || "";
    if (a.length > t) return `${o[0]}.${a.substr(0, t)}`;
    for (; a.length < t;) a += "0";
    return `${o[0]}.${a}`;
}
function padZeroStr(e, t) {
    t = t || 2;
    var o = new Array(t).join("0");
    return (o + e).slice(-t);
}
function textInt(e, t) {
    return isNaN(parseInt(e, 10)) ? t : parseInt(e, 10);
}
function textFloat(e, t) {
    return isNaN(parseFloat(e)) ? t : parseFloat(e);
}
function voyFoVInit() {
    function e() {
        (this.fovWidth = fovdata.fovx),
            (this.fovHeight = fovdata.fovy),
            (this.fovMax = !0),
            (this.fovNotes = !0),
            (this.fovColour = "#ff0000"),
            (this.fovRotation = -fovdata.pAng);
    }
    (this.fovWidth = ""),
        (this.fovHeight = ""),
        (this.fovMax = !0),
        (this.fovNotes = !0),
        (this.fovColour = "#ffffff"),
        (this.fovRotation = 0),
        (this.fovAppH = 0),
        (this.fovAppV = 0),
        (this.fovScopeRes = 0),
        (this.compDesc = ""),
        (this.compDisplay = !0),
        (this.searchTarget = ""),
        (this.searchPopular = ""),
        (this.searchSurveySelection = "P/DSS2/color-m"),
        (this.getFovDataVoy = e);
}
function generateUID() {
    var e,
        t,
        o = "";
    for (t = 0; t < 32; t++)
        (8 != t && 12 != t && 16 != t && 20 != t) || (o += "-"),
            (e = Math.floor(16 * Math.random())
                .toString(16)
                .toUpperCase()),
            (o += e);
    return o;
}
function animatePreDownload() {
    $("#animBar").animate({ opacity: "+=1" }, 500),
        $("#animBar").animate({ opacity: "-=0.9" }, 1e3, animatePreDownload);
}
function populateInfo(e) {
    $(".obj-data-item").remove(),
        e.forEach((e) => {
            $("#obj-data-cont").append(
                '<div class="obj-data-item d-flex align-items-center justify-content-between col py-1"><span class="itemTitle text-muted px-1">' +
                e.Key +
                '</span><span class="itemDescr px-1">' +
                e.Value +
                "</span></div>"
            );
        }),
        $("#cont-detail-obj").removeClass("d-none");
}
function resetField(e) {
    $(e.data.ident).val("");
}
function resetSearchField() {
    $(".searchField").val(""), removeObjectDetail();
}
function removeObjectDetail() {
    $(".obj-data-item").remove(), $("#cont-detail-obj").addClass("d-none");
}
function resetEnvironmentData() {
    $(".env-element-li").remove(),
        $("#cont-seq-list").addClass("d-none"),
        $("#selectedSeq").text(""),
        $("#cont-ds-list").addClass("d-none"),
        $("#selectedDs").text(""),
        stopAnimLogo();
}
function enableActions() {
    voyIsRunning ||
        (btnTempActive.forEach(function (e) {
            if ("" != e.id) {
                var t = "#" + e.id;
                setBtnActive(t);
            }
        }),
            $("body").css("cursor", "default"),
            resetShotPanel());
}
function disableActions() {
    if (voyIsRunning) {
        var e = document.querySelectorAll(".actBtn:not(.disabled)");
        (btnTempActive.length = 0),
            (btnTempActive = [].slice.call(e)),
            btnTempActive.forEach(function (e) {
                if ("" != e.id) {
                    var t = "#" + e.id;
                    setBtnNotActive(t);
                }
            }),
            $("body").css("cursor", "progress");
    }
}
function errorFire(e, t) {
    "" == t && (t = "Error Message!"),
      "" == e && (e = "Undefined error. Trouble with connection?"),
      $("#errorMessage").html(e),
      $("#errorModalTitle").html(t),
      $("#centralModalDanger").modal("show");
  }
function RemoteGetCCDSizeInfoEx() {
    var e = { method: "RemoteGetCCDSizeInfoEx", params: {} };
    (e.params.UID = generateUID()),
        pushUid("RemoteGetCCDSizeInfoEx", e.params.UID),
        doSend(JSON.stringify(e));
}
function RemoteGetCCDSizeInfoReceived(e) {
    (connectedCcdData = e), renderFoV(e);
    let t = document.querySelector("#textSetupAladin");
    (t.innerHTML = "Profile FoV data selected: " + actualProfileName),
        $("#fdb-pAng").val(fovdata.pAng),
        $("#paInd").text(" " + fovdata.pAng + "°"),
        handleLocalStorageProfile(),
        handleProfileAlert();
}
function RemoteGetCCDSizeInfoError(e) {
    errorFire(e);
}
function sendRemoteSolveAndSync() {
    var e = { method: "RemoteSolveActualPosition", params: {} };
    (e.params.UID = generateUID()),
        (e.params.IsBlind = blindsolve),
        (e.params.IsSync = !0),
        pushUid("sendRemoteSolveAndSync", e.params.UID),
        doSend(JSON.stringify(e));
}
function sendRemoteSolveAndSyncReceived(e) {
    $("#solvedStatText").text(e.IsSolved),
        e.IsSolved
            ? $("#solvedStatText")
                .removeClass("text-warning")
                .addClass("text-success")
            : ($("#solvedStatText")
                .removeClass("text-success")
                .addClass("text-warning"),
                $("#solvedStatText").append(" - " + e.LastError));
    let t = e.RA,
        o = e.DEC,
        a = e.PA;
    (a = a.toFixed(1)),
        a < 0 && (a += 360),
        $("#solvRA").text(t),
        $("#solvDEC").text(o),
        $("#solvPA").text(a);
}
function sendRemoteSolveNoSync() {
    var e = { method: "RemoteSolveActualPosition", params: {} };
    (e.params.UID = generateUID()),
        (e.params.IsBlind = blindsolve),
        (e.params.IsSync = !1),
        pushUid("sendRemoteSolveNoSync", e.params.UID),
        doSend(JSON.stringify(e));
}
function sendRemoteSolveNoSyncReceived(e) {
    sendRemoteSolveAndSyncReceived(e);
}
function sendRemoteSolveFov() {
    var e = { method: "RemoteSolveActualPosition", params: {} };
    (e.params.UID = generateUID()),
        (e.params.IsBlind = blindsolve),
        (e.params.IsSync = !1),
        pushUid("sendRemoteSolveFov", e.params.UID),
        doSend(JSON.stringify(e));
}
function sendRemoteSolveFovReceived(e) {
    sendRemoteSolveAndSyncReceived(e);
    let t = e.RA,
        o = e.DEC,
        a = e.PA;
    (a = a.toFixed(1)),
        a < 0 && (a += 360),
        aladin.gotoRaDec(15 * t, o),
        getCoordinateAladin(),
        $("#fdb-pAng").val(a),
        $("#paInd").text(" " + a + "°"),
        changePAng(a),
        $("#inputsearch").val("");
}
function remoteSolveAndSyncError(e, t) {
    errorFire(e + " " + actionResultCodes[t], "Remote action Solve error");
}
function toggleCollapsePlSolvRes() {
    $("#plSolveColl").collapse("toggle");
}
function readJson(e) {
    switch (
    (e.hasOwnProperty("jsonrpc") &&
        0 != e.result &&
        errorFire(e.error + "<br>error code: " + e.code, "JsonRPC error"),
        e.Event)
    ) {
        case "Version":
            VersionRec(e);
            break;
        case "Polling":
            break;
        case "Signal":
            signalReceived(e.Code);
            break;
        case "NewFITReady":
            newFitReadyReceived(e);
            break;
        case "NewJPGReady":
            newJPGReadyReceived(e);
            break;
        case "ShutDown":
            break;
        case "RemoteActionResult":
            remoteActionResultReceived(e);
            break;
        case "ArrayElementData":
            arrayElementDataReceived(e);
            break;
        case "ControlData":
            controlDataReceived(e);
            break;
        case "ShotRunning":
            shotRunningReceived(e);
            break;
        case "LogEvent":
            logEventReceived(e);
            break;
        case "AutoFocusResult":
            autoFocusResultReceived(e);
            break;
        case "ProfileChanged":
            profileChangedReceived(e);
            break;
        default:
            text = "No value found";
    }
    "error" in e && errorFire(e.error.message + " - id:" + e.id);
}
function remoteActionResultReceived(e) {
    var t = actionResultCodes[e.ActionResultInt],
        o = uidList[e.UID];
    "RemoteSetupConnect" === o &&
        4 === e.ActionResultInt &&
        remoteSetupConnected(),
        "RemoteSetupConnect" === o &&
        5 === e.ActionResultInt &&
        remoteSetupError(o, t),
        "RemoteSetupConnect" === o &&
        8 === e.ActionResultInt &&
        remoteSetupTimedOut(),
        "RemoteSetupDisconnect" === o &&
        4 === e.ActionResultInt &&
        remoteSetupDisconnected(),
        "RemoteCameraShot" === o && 4 === e.ActionResultInt && remoteCameraShotOk(),
        "RemoteCameraShot" === o &&
        5 === e.ActionResultInt &&
        remoteCameraShotError(t),
        "RemoteCameraShot" === o &&
        6 === e.ActionResultInt &&
        remoteCameraShotAborting(),
        "RemoteCameraShot" === o &&
        7 === e.ActionResultInt &&
        remoteCameraShotAborted(),
        "RemoteGetFilterConfiguration" === o &&
        4 === e.ActionResultInt &&
        getFilterConfigurationReceived(e),
        "RemoteFilterGetActual" === o &&
        4 === e.ActionResultInt &&
        updateFilterActual(e.ParamRet),
        "RemoteFilterChangeTo" === o &&
        4 === e.ActionResultInt &&
        getActualFilter(),
        "RemoteSearchTarget" === o &&
        4 === e.ActionResultInt &&
        remoteSearchReceived(e.ParamRet),
        "RemoteSearchTarget" === o &&
        5 === e.ActionResultInt &&
        remoteSearchReceivedFinishedError(e.Motivo),
        "RemoteGetEnvironmentData" === o &&
        4 === e.ActionResultInt &&
        remoteEnvironmentDataReceived(e.ParamRet),
        "RemoteGetEnvironmentData" === o &&
        4 !== e.ActionResultInt &&
        errorFire(
            "Remote GetEnvironmentData error Received: " + e.Motivo,
            "Severe error!"
        ),
        "getProfileName" === o &&
        4 === e.ActionResultInt &&
        getProfileNameReceived(e.ParamRet),
        "getProfileName" === o &&
        4 !== e.ActionResultInt &&
        errorFire(
            "Remote Get Profile Name error Received: " + e.Motivo,
            "Severe error!"
        ),
        "RemoteSetLogEvent" === o &&
        4 === e.ActionResultInt &&
        remoteSetLogEventReceived(),
        "RemoteSetDashboardMode" === o &&
        4 === e.ActionResultInt &&
        remoteSetDashBoardModeOk(),
        "RemoteSetDashboardMode" === o &&
        4 !== e.ActionResultInt &&
        errorFire(
            "Remote Set Dashboard Mode Error Received: " + e.Motivo,
            "Severe error!"
        ),
        "RemotePrecisePointTarget" === o &&
        4 === e.ActionResultInt &&
        remotePrecisePointTargetOk(),
        "RemotePrecisePointTarget" === o &&
        5 === e.ActionResultInt &&
        remotePrecisePointTargetError(e.Motivo),
        "RemoteGetListAvalaibleSequence" === o &&
        4 === e.ActionResultInt &&
        getSequenceListReceivedOk(e.ParamRet.list),
        "RemoteGetListAvalaibleSequence" === o &&
        5 === e.ActionResultInt &&
        getSequenceListReceivedError(e.Motivo),
        "RemoteGetListAvalaibleDragScript" === o &&
        4 === e.ActionResultInt &&
        getDsListReceivedOk(e.ParamRet.list),
        "RemoteGetListAvalaibleDragScript" === o &&
        5 === e.ActionResultInt &&
        getDsListReceivedError(e.Motivo),
        "RemoteGetCCDSizeInfoEx" === o &&
        4 === e.ActionResultInt &&
        RemoteGetCCDSizeInfoReceived(e.ParamRet),
        "RemoteGetCCDSizeInfoEx" === o &&
        5 === e.ActionResultInt &&
        RemoteGetCCDSizeInfoError(e.Motivo + " da GetCCDSize"),
        "RemoteMountFastCommand" === o && 4 === e.ActionResultInt
            ? remoteMountFastCommandreceived()
            : "RemoteMountFastCommand" === o &&
            4 != e.ActionResultInt &&
            remoteMountFastCommandError(e.Motivo, e.ActionResultInt),
        "sendRemoteSolveAndSync" === o && 4 === e.ActionResultInt
            ? sendRemoteSolveAndSyncReceived(e.ParamRet)
            : "sendRemoteSolveAndSync" === o &&
            4 != e.ActionResultInt &&
            remoteSolveAndSyncError(e.Motivo, e.ActionResultInt),
        "sendRemoteSolveNoSync" === o && 4 === e.ActionResultInt
            ? sendRemoteSolveNoSyncReceived(e.ParamRet)
            : "sendRemoteSolveNoSync" === o &&
            4 != e.ActionResultInt &&
            remoteSolveAndSyncError(e.Motivo, e.ActionResultInt),
        "sendRemoteSolveFov" === o && 4 === e.ActionResultInt
            ? sendRemoteSolveFovReceived(e.ParamRet)
            : "sendRemoteSolveFov" === o &&
            4 != e.ActionResultInt &&
            remoteSolveAndSyncError(e.Motivo, e.ActionResultInt),
        "RemoteRoboClipGetTargetList" === o && 4 === e.ActionResultInt
            ? RemoteRoboClipGetTargetListReceived(e.ParamRet)
            : ("RemoteRoboClipGetTargetList" === o) & (4 != e.ActionResultInt) &&
            RemoteRoboClipGetTargetListReceivedError(e.Motivo, e.ActionResultInt),
        "RemoteRoboClipRemoveTarget" === o && 4 === e.ActionResultInt
            ? remoteRoboClipRemoveTargetOkReceived(e.ParamRet)
            : ("RemoteRoboClipRemoveTarget" === o) & (4 != e.ActionResultInt) &&
            remoteRoboClipRemoveTargetReceivedError(e.Motivo, e.ActionResultInt),
        "RemoteRoboClipUpdateTarget" === o && 4 === e.ActionResultInt
            ? remoteRoboClipUpdateTargetOkReceived(e.ParamRet)
            : ("RemoteRoboClipUpdateTarget" === o) & (4 != e.ActionResultInt) &&
            remoteRoboClipUpdateTargetReceivedError(e.Motivo, e.ActionResultInt),
        "RemoteRoboClipAddTarget" === o && 4 === e.ActionResultInt
            ? remoteRoboClipAddTargetOkReceived(e.ParamRet)
            : ("RemoteRoboClipAddTarget" === o) & (4 != e.ActionResultInt) &&
            remoteRoboClipAddTargetReceivedError(e.Motivo, e.ActionResultInt),
        "RemoteFocus" === o && 4 === e.ActionResultInt
            ? remoteFocusReceivedOK(e.ParamRet)
            : ("RemoteFocus" === o) & (4 != e.ActionResultInt) &&
            remoteFocusReceivedError(e.Motivo, t),
        "RemoteFocuserMoveTo" === o && 4 === e.ActionResultInt
            ? remoteFocuserMoveReceivedOK(e.ParamRet)
            : ("RemoteFocuserMoveTo" === o) & (4 != e.ActionResultInt) &&
            remoteFocuserMoveReceivedError(e.Motivo, t),
        "RemoteFocusInject" === o && 4 === e.ActionResultInt
            ? RemoteFocusInjectReceivedOK(e.ParamRet)
            : ("RemoteFocusInject" === o) & (4 != e.ActionResultInt) &&
            RemoteFocusInjectReceivedError(e.Motivo, t),
        "RemoteGetVoyagerProfiles" === o && 4 === e.ActionResultInt
            ? remoteGetVoyagerProfilesReceived(e.ParamRet)
            : ("RemoteGetVoyagerProfiles" === o) & (4 != e.ActionResultInt) &&
            remoteGetVoyagerProfilesReceivedError(e.Motivo, t),
        "RemoteGetVoyagerProfilesstartup" === o && 4 === e.ActionResultInt
            ? remoteVoyagerProfileStartupReceived(e.ParamRet)
            : ("RemoteGetVoyagerProfilesstartup" === o) & (4 != e.ActionResultInt) &&
            remoteGetVoyagerProfilesReceivedError(e.Motivo, t),
        "RemoteSetProfile" === o && 4 === e.ActionResultInt
            ? remoteSetProfileReceived(e.ParamRet)
            : ("RemoteSetProfile" === o) & (4 != e.ActionResultInt) &&
            remoteSetProfileReceivedError(e.Motivo, t),
        "RemoteRotatorMoveTo" === o && 4 === e.ActionResultInt
            ? RemoteRotatorMoveToReceived(e.ParamRet)
            : ("RemoteRotatorMoveTo" === o) & (4 != e.ActionResultInt) &&
            rRemoteRotatorMoveToReceivedError(e.Motivo, t),
        removeUid(e.UID);
}
function focusStatsArrayAdd(e) {
    focusStats.length < 10
        ? focusStats.unshift(e)
        : (focusStats.pop(), focusStats.unshift(e)),
        updateFocusStatsTable();
}
function RemoteRoboClipGetTargetListReceivedError(e, t) {
    errorFire(e + " " + actionResultCodes[t], "Remote action RoboClip error");
}
function updateGroupSelect() {
    var e = $("#groupSelect");
    for (
        e.empty(),
        e.append('<option selected="true" disabled>Select Group</option>'),
        i = 0;
        i < roboClipGroup.length;
        i++
    ) {
        var t = roboClipGroup[i];
        e.append($("<option></option>").attr("value", t).text(t));
    }
    0 == roboClipGroup.length &&
        e.append($("<option></option>").attr("value", "").text("No Group"));
}
function resetRcField() {
    $(".rcField").val(""),
        $("#ledMosaicRoboclip").text(""),
        (document.getElementById("rcUid").value = ""),
        enableSaveDelBtn(!1);
}
function actionButtonRoboClip(e) {
    var t = e.currentTarget;
    if (
        ("get" === t.name && (rCgetCoordsAndPa(t.id, !0), showMosListPanel(!1)),
            "edit" === t.name &&
            (rCgetCoordsAndPa(t.id, !1), rCeditRow(t.id), showMosListPanel(!1)),
            "showTiles" === t.name && showTiles(t.id),
            "del" === t.name)
    ) {
        showMosListPanel(!1);
        let e = roboClipTemp.find((e) => e.guid === t.id),
            o = e.targetname;
        modalDoubleWarnFire(
            "Are you sure to remove target: " + o,
            "",
            "delFromRoboClip",
            t.id
        );
    }
}
function resetOrderButtons() {
    $(".orderBtn").removeClass("btn-warning").addClass("btn-light");
}
function setOrderButton() {
    resetOrderButtons();
    var e = "",
        t = "";
    switch (roboClipOrder) {
        case 0:
            (e = "#orderDate"),
                (t = "Target list ordeded by Date Created Descendent");
            break;
        case 1:
            (e = "#orderName"), (t = "Target list ordeded by Name");
            break;
        case 2:
            (e = "#orderGroup"), (t = "Target list ordeded by Group");
            break;
        case 3:
            (e = "#orderRaDesc"), (t = "Target list ordeded by RA Descending");
            break;
        case 4:
            (e = "#orderRaAsc"), (t = "Target list ordeded by RA Ascending");
    }
    "" != e &&
        ($(e).removeClass("btn-light").addClass("btn-warning"),
            $("#rcOrderDescr").text(t));
}
function rCeditRow(e) {
    let t = roboClipTemp.find((t) => t.guid === e),
        o = t.targetname,
        a = coordinateFormat(t.raj2000, "h"),
        n = coordinateFormat(t.decj2000, "º"),
        r = t.pa,
        i = t.note,
        s = t.gruppo;
    $("#rcName").val(o),
        $("#raRc").val(a),
        $("#decRc").val(n),
        $("#paRc").val(r, 2),
        $("#rCnote").val(i),
        chkIfMos()
            ? $("#ledMosaicRoboclip").text(
                "Mosaic [" + mosaicSettings.Hnum + "x" + mosaicSettings.Wnum + "]"
            )
            : $("#ledMosaicRoboclip").text(""),
        (document.getElementById("rcUid").value = e);
    let l = roboClipGroup.findIndex((e) => e === s);
    (document.getElementById("groupSelect").selectedIndex = l + 1),
        showRcAlert("#rcSuccess", "You are now editing an this Target: " + o),
        enableSaveDelBtn(!0);
}
function rCdelRow(e) {
    remoteRoboClipRemoveTarget(e);
}
function resetFilterCont() {
    if ((preResetFilters(), rcFilterSelected)) {
        rcFilterSelected = !1;
        let e = roboClipOrder;
        resetRcField(),
            $("#roboClipModal").modal("hide"),
            setTimeout(function () {
                (roboClipOrder = e), sendRemoteRoboClipGetTargetList();
            }, 500);
    }
}
function preResetFilters() {
    resetFilterVar(),
        $("#filterText").val(""),
        $("#filter-btn-id").text("Select");
}
function rcFilterActivation() {
    let e = $("#filter-btn-id").text(),
        t = $("#filterText").val();
    if ((resetFilterVar(), "Select" != e && "" != t)) {
        let o = roboClipOrder;
        $("#roboClipModal").modal("hide"),
            setTimeout(function () {
                switch (((roboClipOrder = o), (rcFilterSelected = !0), e)) {
                    case "Name":
                        roboClipNameTxt = t;
                        break;
                    case "Group":
                        roboClipGroupTxt = t;
                        break;
                    case "Note":
                        roboClipNoteTxt = t;
                }
                sendRemoteRoboClipGetTargetList();
            }, 500);
    } else
        "" != t &&
            ($("#filter-btn-id").addClass("animated bounceIn fast"),
                $("#filter-btn-id").one(
                    "webkitAnimationEnd mozAnimationEnd MSAnimationEnd oanimationend animationend",
                    function () {
                        $("#filter-btn-id").removeClass("animated bounceIn fast");
                    }
                ));
}
function resetFilterVar() {
    (roboClipNameTxt = ""), (roboClipGroupTxt = ""), (roboClipNoteTxt = "");
}
function reloadAfterFilterSelectEmpty() {
    var e = 10,
        t = setInterval(function () {
            (document.getElementById("progressBarReload").value = e),
                (document.getElementById("timerText").innerText = e + " sec"),
                (e -= 1),
                e <= 0 && (clearInterval(t), resetFilterCont());
        }, 1e3);
}
function frameFoV(e, t, o, a, n, r, i, s, l, c) {
    function d() {
        if (
            ((f = this.aLinst.ctx[this.fdbgi]),
                (this.animId = fovReqAnim(d.bind(this))),
                this.aLinst.getFov()[0] != this.lastZoom ||
                this.aLinst.getSize()[0] != this.lastSize[0] ||
                this.aLinst.getSize()[1] != this.lastSize[1] ||
                this.forceRedraw)
        ) {
            (this.forceRedraw = !1),
                (f.font = "10px Roboto"),
                (f.fillStyle = "#E678FA"),
                f.clearRect(
                    -f.canvas.width / 2 - 1,
                    -f.canvas.height / 2 - 1,
                    f.canvas.width + 2,
                    f.canvas.height + 2
                ),
                (this.aLinst.getSize()[0] == this.lastSize[0] &&
                    this.aLinst.getSize()[1] == this.lastSize[1]) ||
                (this.aLinst.resizeCanvas(this.fdbgi), this.autoZoom()),
                this.aZoomOn &&
                (this.forceZoom > 0
                    ? ($(".zoomIn-container, .zoomOut-container").css(
                        "display",
                        "none"
                    ),
                        $(".bws-maxIndicator").css("display", "block"),
                        this.forceZoom <= 90
                            ? (this.aLinst.setFOVRange(null, null),
                                this.aLinst.progressiveFov(this.forceZoom, !1),
                                this.aLinst.setFOVRange(
                                    this.aLinst.getFov()[0],
                                    this.aLinst.getFov()[0]
                                ))
                            : (this.aLinst.setFOVRange(null, null),
                                this.aLinst.progressiveFov(90, !0),
                                this.aLinst.setFOVRange(
                                    this.aLinst.getFov()[0],
                                    this.aLinst.getFov()[0]
                                )))
                    : ($(".zoomIn-container, .zoomOut-container").css(
                        "display",
                        "block"
                    ),
                        $(".bws-maxIndicator").css("display", "none"),
                        this.aLinst.setFOVRange(null, null),
                        this.aLinst.getFov()[0] > 90 &&
                        (this.aLinst.setFov(1), this.aLinst.progressiveFov(90, !0)),
                        this.aLinst.setFOVRange(0, 90)));
            var e =
                Math.round(aladin.getSize()[1] / 2) /
                (aladin.pix2world(
                    Math.round(aladin.getSize()[0] / 2) - 1,
                    Math.round(aladin.getSize()[1] / 2)
                )[1] -
                    aladin.pix2world(
                        Math.round(aladin.getSize()[0] / 2) - 1,
                        aladin.getSize()[1] - 1
                    )[1]),
                t = this.vFoVwidth * e,
                o = this.vFoVheight * e;
            let d = "rgba(255, 255, 255, .06)";
            f.rotate(this.vFoVrotat.toRad()),
                (f.strokeStyle = this.frameColor),
                (f.fillStyle = d),
                (f.font = "14px Roboto"),
                f.setLineDash([]),
                (mosaicTiles = []);
            let x = t / mosaicSettings.Wnum,
                y = o / mosaicSettings.Hnum,
                D = Math.max(x, y),
                T = (D * mosaicSettings.Overlap) / 100,
                P = (D * mosaicSettings.Overlap) / 100,
                w = T * (mosaicSettings.Wnum - 1),
                E = P * (mosaicSettings.Hnum - 1);
            if (
                (f.canvas.clientWidth,
                    f.canvas.clientHeight,
                    mosaicSettings.Wnum > 1 || mosaicSettings.Hnum > 1)
            ) {
                let e = 1;
                for (var a = 0; a < mosaicSettings.Hnum; a++)
                    for (var n = 0; n < mosaicSettings.Wnum; n++) {
                        f.setLineDash([8, 3]), (f.lineWidth = "1");
                        let s = n * (x - T) - (t - w) / 2,
                            l = a * (y - P) - (o - E) / 2;
                        f.save(), f.translate(s + x / 2, l + y / 2);
                        let c = 0 - x / 2,
                            m = 0 - y / 2,
                            u = {};
                        (u.x = Math.round(10 * f.getTransform().e) / 10),
                            (u.y = Math.round(10 * f.getTransform().f) / 10);
                        let p = calculateSingleRot(u),
                            g = 0,
                            v = fovdata.pAng + p;
                        v < 0 && (v += 360),
                            mosaicSettings.angleAdj || ((g = p), (v = fovdata.pAng)),
                            f.rotate((g * Math.PI) / 180),
                            f.strokeRect(c, m, x, y),
                            f.fillRect(c, m, x, y);
                        let h = aladin.pix2world(u.x, u.y),
                            S = Math.round(10 * v) / 10;
                        360 == S && (S = 0);
                        let C = {
                            x: u.x,
                            y: u.y,
                            raHours: h[0] / 15,
                            decDegr: h[1],
                            ra: coordinateFormatClear(h[0] / 15),
                            dec: coordinateFormatClear(h[1]),
                            singlePA: S
                        };
                        (mosaicTiles[e - 1] = C),
                            f.setLineDash([]),
                            f.beginPath(),
                            f.moveTo(0, -12);
                        var r = 0,
                            i = -y / 2;
                        f.lineTo(r, i),
                            f.stroke(),
                            f.restore(),
                            f.setLineDash([]),
                            (f.fillStyle = "rgba(15, 71, 255, 0.5)"),
                            f.strokeText(
                                e,
                                n * (x - T) - (t - w) / 2 + x / 2 - 5,
                                a * (y - P) - (o - E) / 2 + y / 2 + 5
                            ),
                            (f.fillStyle = d),
                            e++,
                            f.restore();
                    }
            }
            (f.lineWidth = "1"),
                f.setLineDash([]),
                1 == mosaicSettings.Wnum && 1 == mosaicSettings.Hnum
                    ? (f.strokeRect(-t / 2, -o / 2, t, o),
                        f.fillRect(-t / 2, -o / 2, t, o))
                    : ((t -= w), (o -= E));
            let N = 2,
                L = 60,
                k = f,
                B = -t / 2 - N + 1,
                M = -o / 2 - N + 1,
                O = -1 * B,
                U = -1 * M,
                V = -1 * B,
                G = M,
                H = B,
                q = U;
            (f.lineWidth = N),
                k.beginPath(),
                k.moveTo(B + L, M),
                k.lineTo(B, M),
                k.lineTo(B, M + L),
                k.moveTo(O - L, U),
                k.lineTo(O, U),
                k.lineTo(O, U - L),
                k.moveTo(V - L, G),
                k.lineTo(V, G),
                k.lineTo(V, G + L),
                k.moveTo(H + L, q),
                k.lineTo(H, q),
                k.lineTo(H, q - L),
                k.stroke(),
                (f.strokeStyle = this.frameColor),
                (f.fillStyle = this.frameColor),
                (f.lineWidth = "1"),
                t > 135 &&
                ((f.font = "11px Roboto"),
                    f.fillText("PA " + fovdata.pAng + "°", t / 2 - 58, o / 2 - 8)),
                handleProfileAlert();
            let z = "";
            if (t > 135) {
                (f.font = "11px Roboto"), sameFov || (f.fillStyle = "#fb3");
                let e = z + "Fov Profile: " + actualProfileName;
                f.fillText(e, -t / 2 + 10, o / 2 - 8);
            }
            if (
                ((f.fillStyle = this.frameColor),
                    this.annotateOvl &&
                    t > 142 &&
                    1 == mosaicSettings.Wnum &&
                    1 == mosaicSettings.Hnum)
            ) {
                (f.font = t > 249 ? "12px Roboto" : "10px Roboto"),
                    f.fillText(
                        parseFloat(this.vFoVwidth).toFixed(2) +
                        "° ( " +
                        (60 * parseFloat(this.vFoVwidth)).toFixed(1) +
                        "' )",
                        -t / 2 + 45,
                        -o / 2 + 15
                    ),
                    f.beginPath();
                var s = -t / 2,
                    l = -o / 2 + 11,
                    c = s + 38,
                    m = l,
                    u = 6,
                    p = c - s,
                    g = m - l,
                    v = Math.atan2(g, p);
                f.moveTo(s, l),
                    f.lineTo(c, m),
                    f.lineTo(
                        c - u * Math.cos(v - Math.PI / 6),
                        m - u * Math.sin(v - Math.PI / 6)
                    ),
                    f.moveTo(c, m),
                    f.lineTo(
                        c - u * Math.cos(v + Math.PI / 6),
                        m - u * Math.sin(v + Math.PI / 6)
                    ),
                    f.stroke(),
                    0 !== this.winOriz &&
                    f.fillText(
                        parsefloat(this.winOriz).toFixed(2) + '" per pixel',
                        -t / 2 + 50,
                        -o / 2 + 26
                    ),
                    0 !== this.scopeRes &&
                    f.fillText(
                        "Dawes Limit: " + (1 * this.scopeRes).toFixed(2) + '"',
                        -t / 2 + 45,
                        -o / 2 + 39
                    );
            }
            if (
                this.annotateOvl &&
                o > 123 &&
                1 == mosaicSettings.Wnum &&
                1 == mosaicSettings.Hnum
            ) {
                (f.font = t > 249 ? "12px Roboto" : "10px Roboto"),
                    f.rotate((90).toRad()),
                    f.fillText(
                        parseFloat(this.vFoVheight).toFixed(2) +
                        "° ( " +
                        (60 * parseFloat(this.vFoVheight)).toFixed(1) +
                        "' )",
                        -o / 2 + 45,
                        t / 2 - 7
                    ),
                    0 !== this.winVert &&
                    f.fillText(
                        this.winVert.toFixed(2) + '" per pixel.',
                        -o / 2 + 40,
                        t / 2 - 18
                    ),
                    f.beginPath();
                var h = -o / 2,
                    S = t / 2 - 11,
                    C = h + 38,
                    R = S,
                    b = 6,
                    A = C - h,
                    F = R - S,
                    I = Math.atan2(F, A);
                f.moveTo(h, S),
                    f.lineTo(C, R),
                    f.lineTo(
                        C - b * Math.cos(I - Math.PI / 6),
                        R - b * Math.sin(I - Math.PI / 6)
                    ),
                    f.moveTo(C, R),
                    f.lineTo(
                        C - b * Math.cos(I + Math.PI / 6),
                        R - b * Math.sin(I + Math.PI / 6)
                    ),
                    f.stroke(),
                    f.rotate(-(90).toRad());
            }
            this.annotateOvl
                ? this.aLinst.view.showReticle(!0)
                : this.aLinst.view.showReticle(!1),
                f.rotate(-this.vFoVrotat.toRad()),
                (this.lastZoom = this.aLinst.getFov()[0]),
                (this.lastSize[0] = this.aLinst.getSize()[0]),
                (this.lastSize[1] = this.aLinst.getSize()[1]);
        }
    }
    function m() {
        "unset" != this.animId &&
            (fovCanAnim(this.animId),
                (this.animId = "unset"),
                f.clearRect(
                    -f.canvas.width / 2 - 1,
                    -f.canvas.height / 2 - 1,
                    f.canvas.width + 2,
                    f.canvas.height + 2
                ),
                this.aZoomOn &&
                ($(".aladin-location").css("display", "block"),
                    $(".zoomIn-container, .zoomOut-container").css("display", "block"),
                    $(".bws-maxIndicator").css("display", "none"),
                    this.aLinst.setFOVRange(null, null),
                    this.aLinst.view.showReticle(!0)));
    }
    function u() {
        if (this.maxOvl) {
            var e = legatoH(
                -this.vFoVwidth / 2,
                -this.vFoVheight / 2,
                this.vFoVwidth / 2,
                -this.vFoVheight / 2,
                -this.vFoVwidth / 2,
                this.vFoVheight / 2,
                this.vFoVwidth / 2,
                this.vFoVheight / 2,
                this.vFoVrotat
            ),
                t = legatoW(
                    -this.vFoVwidth / 2,
                    -this.vFoVheight / 2,
                    this.vFoVwidth / 2,
                    -this.vFoVheight / 2,
                    -this.vFoVwidth / 2,
                    this.vFoVheight / 2,
                    this.vFoVwidth / 2,
                    this.vFoVheight / 2,
                    this.vFoVrotat
                ),
                o = this.aLinst.getSize()[0] / this.aLinst.getSize()[1];
            this.forceZoom = Math.max(1.03 * t, e * o * 1.03);
        } else this.forceZoom = 0;
    }
    function p() {
        this.aZoomOn = !0;
    }
    function g() {
        null !== this.aLinst &&
            ($(".aladin-location").css("display", "block"),
                $(".zoomIn-container, .zoomOut-container").css("display", "block"),
                $(".bws-maxIndicator").css("display", "none"),
                this.aLinst.setFOVRange(null, null),
                this.aLinst.view.showReticle(!0)),
            (this.aZoomOn = !1);
    }
    function v(e) {
        this.stopDrw(),
            (this.aLinst = e),
            this.autoZoom(),
            (this.forceRedraw = !0),
            this.actDrw();
    }
    (this.vFoVwidth = e),
        (this.vFoVheight = t),
        (this.vFoVrotat = o),
        (this.frameColor = a),
        (this.winOriz = n),
        (this.winVert = r),
        (this.scopeRes = i),
        (this.maxOvl = s),
        (this.annotateOvl = l),
        (this.description = c),
        (this.fdbgi = "fdbFovFrame"),
        (this.aLinst = null),
        (this.animId = "unset"),
        (this.forceRedraw = !0),
        (this.lastSize = [0, 0]),
        (this.forceZoom = 0),
        (this.lastZoom = 0),
        (this.aZoomOn = !1),
        (this.actDrw = d),
        (this.stopDrw = m),
        (this.autoZoom = u),
        (this.sendFrg = p),
        (this.sndBk = g),
        (this.vFovDraw = v);
    let f = null;
}
function legatoW(e, t, o, a, n, r, i, s, l) {
    var c = Math.cos(l.toRad()),
        d = Math.sin(l.toRad()),
        m = e * c + t * d,
        u = o * c + a * d,
        p = n * c + r * d,
        g = i * c + s * d;
    return Math.max(m, u, p, g) - Math.min(m, u, p, g);
}
function legatoH(e, t, o, a, n, r, i, s, l) {
    var c = Math.cos(l.toRad()),
        d = Math.sin(l.toRad()),
        m = e * d + t * c,
        u = o * d + a * c,
        p = n * d + r * c,
        g = i * d + s * c;
    return Math.max(m, u, p, g) - Math.min(m, u, p, g);
}
function vFoVStart() {
    vFoVdata[vFoVfrm].getFovDataVoy(),
        (vFoVdata[vFoVfrm].compDisplay = !0),
        (virtualFoV[vFoVfrm].vFoVwidth =
            vFoVdata[vFoVfrm].fovWidth * mosaicSettings.Wnum),
        (virtualFoV[vFoVfrm].vFoVheight =
            vFoVdata[vFoVfrm].fovHeight * mosaicSettings.Hnum),
        (virtualFoV[vFoVfrm].vFoVrotat = vFoVdata[vFoVfrm].fovRotation),
        (virtualFoV[vFoVfrm].maxOvl = vFoVdata[vFoVfrm].fovMax),
        (virtualFoV[vFoVfrm].annotateOvl = vFoVdata[vFoVfrm].fovNotes),
        (virtualFoV[vFoVfrm].frameColor = vFoVdata[vFoVfrm].fovColour),
        (virtualFoV[vFoVfrm].winOriz = vFoVdata[vFoVfrm].fovAppH),
        (virtualFoV[vFoVfrm].winVert = vFoVdata[vFoVfrm].fovAppV),
        (virtualFoV[vFoVfrm].scopeRes = vFoVdata[vFoVfrm].fovScopeRes),
        (virtualFoV[vFoVfrm].description = vFoVdata[vFoVfrm].compDesc),
        virtualFoV[vFoVfrm].sendFrg(),
        virtualFoV[vFoVfrm].vFovDraw(aladin);
}
function animLogo() {
    logoanimationaccepted
        ? (tlLoopAnim.restart(), tlLoopAnim2.restart())
        : ($(".st0").css({ fill: "red", transition: "1.0s" }),
            tlStartAnim3.restart());
}
function stopAnimLogo() {
    logoanimationaccepted
        ? (tlLoopAnim.pause(),
            tlLoopAnim2.pause(),
            tlcloseAnim.play(),
            tlCloseAnim2.play())
        : ($(".st0").css({ fill: "#4285F4", transition: ".5s" }),
            tlCloseAnim3.play());
}
var ready = (e) => {
    "loading" != document.readyState
        ? e()
        : document.addEventListener("DOMContentLoaded", e);
};
ready(() => {
    initAll();
});
var internetconnected = !1,
    imgShooting = !1,
    startupConnected = !1,
    btnToInitActivate = [
        "#connectBtn",
        ".modal-voy-btn",
        ".reset-btn",
        ".menuBtn"
    ],
    lev1Btns = [".lev1active"],
    simulatorMode = !1,
    simbadPointer = !1,
    blindsolve = !1,
    debugMode = !1,
    cameraFilterList = [],
    voyIsRunning = !1,
    actionToConfirm = "",
    btnTempActive = [],
    imageStats = [],
    focusStats = [],
    windowWidth = 0,
    remoteLogLevel = 0,
    genStatusText = "",
    runSeq = "",
    runDs = "",
    fovdata = {},
    actualCcdData = {},
    connectedCcdData = {},
    sameFov = !0,
    connectedProfileName,
    multiFov = {
        Virtual_Fov_Default: {
            DX: 3358,
            DY: 2536,
            PixelSize: 5.4,
            PA: 0,
            Focallen: 530
        }
    },
    mosaicSettings = { Wnum: 1, Hnum: 1, Overlap: 15, angleAdj: !1 },
    mosaicTiles = [],
    actualProfileName = "",
    roboClipGroup = [],
    roboClipTemp = [],
    roboClipOrder = 0,
    roboClipNameTxt = "",
    roboClipGroupTxt = "",
    roboClipNoteTxt = "",
    rcFilterSelected = !1,
    mountConnected = !1,
    ccdConnected = !1,
    ccdCoolerOn = !1,
    planConnected = !1,
    plateSolveConnected = !1,
    guideConnected = !1,
    autofocusConnected = !1,
    mountTracking = !1,
    mountParked = !1,
    mountFlip = 0,
    mountSlew = !1,
    ccdStatus = 0,
    connected = !1,
    pollingInterval = 5,
    str_end = "\r\n";
(rotIsRotating = !1),
    (rotConnected = !1),
    (rotPA = 0),
    (rotSkyPa = 0),
    (rotatorPanelHidden = !1);

var aladin,
    ua = window.navigator.userAgent,
    iOS = !!ua.match(/iPad/i) || !!ua.match(/iPhone/i),
    webkit = !!ua.match(/WebKit/i),
    iOSSafari = iOS && webkit && !ua.match(/CriOS/i);
iOSSafari
    ? $("#fsBtn").addClass("d-none")
    : $("#fsBtn").click(() => {
        switchFullscreen();
    }),
    $("#collapseAllBtn").click(() => {
        collapseAllPanels();
    }),
    (collapseAllPanels = (e) => {
        $(".card-body").slideUp(300, function () {
            $(".collapseBtn").html('<img src="img/expandIcn.png" alt="xpand">'),
                closeMiniMenu(),
                goToTopPage();
        });
    }),
    (expandAllPanels = () => {
        $(".card-body").slideDown(0, function () {
            $(".collapseBtn").html('<img src="img/collapseIcn.png" alt="collapse">');
        });
    }),
    (expandPanelByHeaderId = (e) => {
        $(e).click();
    }),
    (saveLocalStorage = (e) => {
        "" != e && localStorage.setItem("host", e);
    }),
    (getLocalStorageIp = () => {
        var e = localStorage.getItem("host");
        (document.getElementById("ipAddress").value = e),
            checkParamPort(),
            checkParamWssSwitch();
    }),
    (initAladin = () => {
        var e = window.navigator.onLine;
        e
            ? renderAladin()
            : ($("#aladinRow").hide(),
                errorFire("Without internet connection Virtual FOV Panel is hidden."));
    }),
    (createframeFoV = (e, t, o, a, n, r, i, s, l) => {
        (vFoVfrm = virtualFoV.length),
            (virtualFoV[vFoVfrm] = new frameFoV(e, t, o, a, n, r, i, s, l)),
            (vFoVdata[vFoVfrm] = new voyFoVInit()),
            aladin.addCanvas(virtualFoV[vFoVfrm].fdbgi),
            vFoVStart();
    }),
    (coordinateFormat = (e, t) => {
        var o = 0 > e;
        e = parseFloat(3600 * e, 10) / 3600;
        var a = parseInt(e, 10),
            n = 60 * Math.abs(e - a),
            r = parseInt(n, 10),
            i = 60 * (n - r),
            s = parseFloat(i.toFixed(3));
        return `${o && 0 === a ? "-" : ""
            }${a.zeropad()}${t} ${r.zeropad()}' ${s.zeropad()}"`;
    }),
    (coordinateFormatClear = (e) => {
        var t = 0 > e;
        e = parseFloat(3600 * e, 10) / 3600;
        var o = parseInt(e, 10),
            a = 60 * Math.abs(e - o),
            n = parseInt(a, 10),
            r = 60 * (a - n),
            i = parseFloat(r.toFixed(3));
        return `${t && 0 === o ? "-" : ""
            }${o.zeropad()} ${n.zeropad()} ${i.zeropad()}`;
    }),
    (changePAng = (e) => {
        (fovdata.pAng = Math.round(100 * parseFloat(e)) / 100),
            fovdata.pAng < 0 && (fovdata.pAng = fovdata.pAng + 360),
            (vFoVdata[vFoVfrm].fovRotation = fovdata.pAng),
            vFoVdata[vFoVfrm].fovRotation < 0 &&
            (vFoVdata[vFoVfrm].fovRotation = vFoVdata[vFoVfrm].fovRotation + 360),
            saveLocalStorageAlad(),
            $("#paInd").text(" " + fovdata.pAng + "°"),
            $("#fdb-pAng").val(fovdata.pAng),
            vFoVStart();
    }),
    (saveLocalStorageAlad = () => {
        (fovdata.profileName = actualProfileName),
            localStorage.setItem("fovdata", JSON.stringify(fovdata));
    }),
    (handleLocalStorageProfile = () => {
        (multiFov[actualProfileName] = actualCcdData),
            setLocalStorageMultiFov(),
            populateDdProfileMenu();
    }),
    (getLocalStorageAlad = () => {
        var e = JSON.parse(localStorage.getItem("fovdata"));
        "fovdata" in localStorage
            ? ((fovdata = e),
                (actualProfileName = "Last_Used"),
                (actualCcdData.DX = fovdata.DX),
                (actualCcdData.DY = fovdata.DY),
                (actualCcdData.PixelSize = fovdata.pixsize),
                (actualCcdData.PA = fovdata.pAng),
                (actualCcdData.Focallen = fovdata.focal),
                handleLocalStorageProfile(),
                $("#textSetupAladin").text(
                    "Sensor and Telescope data retreived from last used."
                ))
            : ((actualCcdData = multiFov.Virtual_Fov_Default),
                (actualProfileName = "Virtual_Fov_Default"),
                $("#textSetupAladin").text(
                    "Sensor and Telescope data not avaiable, default data loaded."
                )),
            populateAladinfield(),
            $("#fdb-pAng").val(fovdata.pAng),
            $("#paInd").text(" " + fovdata.pAng + "°");
    }),
    (populateAladinfield = () => {
        $("#setupReso").text(fovdata.reso + '"/px'),
            $("#setupFocal").text(fovdata.focal + " mm");
        var e = (60 * fovdata.fovx).toFixed(1),
            t = (60 * fovdata.fovy).toFixed(1);
        $("#fovXsize").text(fovdata.fovx + " ° (" + e + "')"),
            $("#fovYsize").text(fovdata.fovy + " ° (" + t + "')"),
            $("#chipXsize").text(fovdata.xsize + " mm"),
            $("#chipYsize").text(fovdata.ysize + " mm"),
            $("#chipXsizePx").text(fovdata.DX),
            $("#chipYsizePx").text(fovdata.DY),
            $("#pixSize").text(fovdata.pixsize),
            checkFovIncompleteData(fovdata.DX, fovdata.DY);
    }),
    (checkFovIncompleteData = (e, t) => {
        if (aladin)
            if (0 === e || 0 === t) {
                const e = "Incomplete sensor data,",
                    t = "please connect to Voyager environment",
                    n = "or change profile",
                    r = 85;
                aladin.addCanvas("canvasAlert");
                var o = document.getElementById("canvasAlert");
                o.style.zIndex = 2;
                var a = o.getContext("2d");
                (a.textAlign = "center"),
                    (a.textBaseline = "middle"),
                    (a.fillStyle = "red"),
                    (a.font = "bold 15px Arial"),
                    a.fillText(e.toUpperCase(), 0, r),
                    a.fillText(t.toUpperCase(), 0, r + 20),
                    a.fillText(n.toUpperCase(), 0, r + 40);
            } else {
                var n = document.getElementById("canvasAlert");
                void 0 !== n && null != n && aladin.removeCanvas("canvasAlert");
            }
    }),
    (aladinFindOnReturn = (e) =>
        (13 != event.which && 13 != event.keyCode) ||
        ($("#search_com").trigger("click"), !1)),
    (sendRaDecToFov = (e, t) => {
        var o = convertRainDecimal(e),
            a = convertDecinDecimal(t);
        aladin.gotoRaDec(o, a), getCoordinateAladin();
    }),
    $("#simulatorSelected").click(function () {
        mountConnected && (simulatorMode = this.checked), adaptFovAsset();
    }),
    $(".blindSwitchInput").click(function () {
        (blindsolve = this.checked),
            $(".blindSwitchInput").prop("checked", blindsolve);
    }),
    (adaptFovAsset = () => { }),
    (convertRainDecimal = (e) => {
        var t = e.split(/[^\d\w]+/);
        t[2] = parseFloat(t[2]) + parseFloat("0." + t[3]);
        var o =
            15 * (textFloat(t[0]) + textFloat(t[1] / 60) + textFloat(t[2] / 3600));
        return o;
    }),
    (convertDecinDecimal = (e) => {
        var t = e.substr(0, 1),
            o = e.substr(1, e.length),
            a = o.split(/[^\d\w]+/);
        a[2] = parseFloat(a[2]) + parseFloat("0." + a[3]);
        var n = textFloat(a[0]) + textFloat(a[1] / 60) + textFloat(a[2] / 3600);
        return t + n;
    }),
    (reticleMouseUpAladin = () => {
        getCoordinateAladin(),
            setTimeout(reDrawVfov, 500),
            simulatorMode && $("#simulatorSelected").trigger("click");
    }),
    $("#getTelescopeCoord").click(function () {
        mountConnected &&
            (sendRaDecToFov(tempRaRec, tempDecRec), setTimeout(reDrawVfov, 2e3));
    }),
    $("#sendCoordToFov").click(function () {
        var e = $("#raSearch").val(),
            t = $("#decSearch").val();
        "" !== e
            ? sendRaDecToFov(e, t)
            : errorFire(
                "Please insert RA and DEC J2000 coordinates or make a Search before send to Virtual Fov.",
                "Empty coords field"
            );
    }),
    $("#sendToFovSolveResult").click(function () {
        var e = $("#solvRA").text(),
            t = $("#solvDEC").text(),
            o = $("#solvPA").text();
        "" !== e
            ? (aladin.gotoRaDec(15 * e, t),
                getCoordinateAladin(),
                $("#paInd").text(" " + o + "°"),
                changePAng(o),
                $("#inputsearch").val(""))
            : errorFire(
                "Please complete a Plate Solve before send to Virtual Fov.",
                "Empty PlateSolve Data"
            );
    }),
    (dateconversion = (e) => {
        var t = new Date(0);
        return t.setUTCSeconds(e), t;
    }),
    $("#getDataRcFov").click(getCoordinateAladinRoboClip),
    $("#groupSelect").on("change", function () {
        $("#newGroup").val("");
    }),
    $("#newGroup").on("keyup", function () {
        $("#groupSelect").val("");
    }),
    $("#newGroup").on("change", function () {
        $("#groupSelect").val("");
    }),
    (Number.prototype.zeropad = function () {
        let e = this;
        return (
            e < 10 && e > 0 && (e = "0" + e),
            0 == e && (e = "0" + e),
            e < 0 && e > -9 && (e = "-0" + e.toString().slice(-1)),
            e
        );
    }),
    (setClipboardData = (e) => {
        let t = coordinateFormatClear(e[0] / 15),
            o = coordinateFormatClear(e[1]),
            a = t + ";" + o;
        document
            .querySelector("#alaCopyBtn")
            .setAttribute("data-clipboard-text", a);
    });
var coordClipB = new ClipboardJS("#alaCopyBtn");
coordClipB.on("success", function (e) {
    $("#alaCopyBtn").tooltip("hide"), e.clearSelection();
}),
    coordClipB.on("error", function (e) {
        errorFire("Clipboard error " + e.trigger, "Clipboard error");
    }),
    (setClipboardMos = () => {
        let e = getMosaicCsvText();
        document
            .querySelector("#mosaCopyBtn")
            .setAttribute("data-clipboard-text", e);
    }),
    $("#mosaCopyBtn").click(function () {
        setClipboardMos();
    });
var mosClipB = new ClipboardJS("#mosaCopyBtn");
mosClipB.on("success", function (e) {
    $("#mosaCopyBtn").tooltip("hide"), e.clearSelection();
}),
    mosClipB.on("error", function (e) {
        errorFire("Clipboard error " + e.trigger, "Clipboard error");
    }),
    ($.fn.modal.Constructor.prototype._enforceFocus = function () { });
var mosClipBlist = new ClipboardJS("#copyMosList");
(setClipboardMosList = (e) => {
    let t = e;
    document.querySelector("#copyMosList").setAttribute("data-clipboard-text", t);
}),
    mosClipBlist.on("success", function (e) {
        $("#copyMosList").tooltip("hide"), e.clearSelection();
    }),
    mosClipBlist.on("error", function (e) {
        errorFire("Clipboard error " + e.trigger, "Clipboard error");
    }),
    $(".logoAnimationSwitch").click(function () {
        voyIsRunning
            ? $(".logoAnimationSwitch").prop("checked", !logoanimationaccepted)
            : ((logoanimationaccepted = !this.checked),
                $(".logoAnimationSwitch").prop("checked", !logoanimationaccepted),
                saveLocalStorageAnimPref());
    }),
    (saveLocalStorageAnimPref = () => {
        null !== logoanimationaccepted &&
            localStorage.setItem("animationlogo", logoanimationaccepted);
    }),
    (getLocalStorageAnimPref = () => {
        let e = JSON.parse(localStorage.getItem("animationlogo"));
        (logoanimationaccepted = null !== e && e),
            $(".logoAnimationSwitch").prop("checked", !logoanimationaccepted);
    }),
    (resetLocalStorageData = () => {
        localStorage.clear(), window.location.reload(!0);
    }),
    (getLocalStorageMultiFov = () => {
        null !== localStorage.getItem("multiFov") &&
            ((multiFov = JSON.parse(localStorage.getItem("multiFov"))),
                populateDdProfileMenu());
    }),
    (setLocalStorageMultiFov = () => {
        localStorage.setItem("multiFov", JSON.stringify(multiFov));
    }),
    (populateDdProfileMenu = () => {
        let e = document.querySelector("#profileMenuSelId"),
            t = document.querySelector("#textSetupAladin");
        for (var o in ((e.innerHTML = ""), multiFov))
            if (multiFov.hasOwnProperty(o)) {
                let t = document.createElement("div");
                (t.classList = "dropdown-item drop-profile"),
                    t.setAttribute("data-value", o);
                let a = document.createTextNode(o);
                t.appendChild(a), e.appendChild(t);
            }
        document.querySelectorAll(".drop-profile").forEach((e) => {
            e.addEventListener("click", function (e) {
                handleProfileAlert();
                let o = e.currentTarget.getAttribute("data-value");
                (actualProfileName = o),
                    renderFoV(multiFov[o]),
                    (t.innerHTML = "Profile FoV data selected: " + actualProfileName);
            });
        });
    }),
    (reDrawVfov = () => {
        (mosaicSettings.Wnum > 1 || mosaicSettings.Hnum > 1) &&
            virtualFoV[vFoVfrm].vFovDraw(aladin);
    }),
    (firstDelayedRedraw = () => {
        setTimeout(reDrawVfov, 2e3);
    }),
    document
        .getElementById("fdb-mosOverlap")
        .addEventListener("input", function (e) {
            setOverlap(this.value);
        }),
    (document.getElementById("mosWnum").onchange = (e) => {
        (mosaicSettings.Wnum = e.srcElement.value), updateMosaicTiles();
    }),
    (document.getElementById("mosHnum").onchange = (e) => {
        (mosaicSettings.Hnum = e.srcElement.value), updateMosaicTiles();
    }),
    (setOverlap = (e) => {
        (document.getElementById("overlapVal").textContent = e + " %"),
            (mosaicSettings.Overlap = e),
            reDrawVfov(),
            saveLocalStorageMosaic();
    }),
    (saveLocalStorageMosaic = () => {
        localStorage.setItem("mosdata", JSON.stringify(mosaicSettings));
    }),
    (getLocalStorageMosaic = () => {
        let e = JSON.parse(localStorage.getItem("mosdata"));
        "mosdata" in localStorage && (mosaicSettings = e), updateMosaicInterface();
    }),
    (updateMosaicTiles = () => {
        vFoVStart(), saveLocalStorageMosaic();
    }),
    $(".advancedAngleSwitchInput").click(function () {
        (mosaicSettings.angleAdj = this.checked),
            $(".advancedAngleSwitchInput").prop("checked", mosaicSettings.angleAdj),
            updateMosaicTiles();
    }),
    (document.getElementById("resetMOS").onclick = (e) => {
        resetMosaic();
    }),
    (resetMosaic = () => {
        (mosaicSettings.Hnum = 1),
            (mosaicSettings.Wnum = 1),
            (mosaicSettings.Overlap = 15),
            (mosaicSettings.angleAdj = !1),
            updateMosaicTiles(),
            updateMosaicInterface();
    }),
    (setNewValuesMosaic = (e, t, o, a) => {
        (mosaicSettings.Hnum = e),
            (mosaicSettings.Wnum = t),
            (mosaicSettings.Overlap = o),
            (mosaicSettings.angleAdj = JSON.parse(a)),
            updateMosaicTiles(),
            updateMosaicInterface();
    }),
    (updateMosaicInterface = () => {
        (document.getElementById("fdb-mosOverlap").value = mosaicSettings.Overlap),
            (document.getElementById("overlapVal").textContent =
                mosaicSettings.Overlap + " %"),
            (document.getElementById("mosWnum").value = mosaicSettings.Wnum),
            (document.getElementById("mosHnum").value = mosaicSettings.Hnum),
            $(".advancedAngleSwitchInput").prop("checked", mosaicSettings.angleAdj);
    }),
    (handleProfileAlert = () => {
        null != connectedProfileName &&
            (actualCcdData.DX == connectedCcdData.DX &&
                actualCcdData.DY == connectedCcdData.DY &&
                actualCcdData.PixelSize == connectedCcdData.PixelSize &&
                actualCcdData.Focallen == connectedCcdData.Focallen
                ? ((sameFov = !0), showHideAlertProfile())
                : ((sameFov = !1), showHideAlertProfile()));
    }),
    (showHideAlertProfile = () => {
        sameFov
            ? (document.getElementById("alertFovDiv").classList.remove("d-block"),
                document.getElementById("alertFovDiv").classList.add("d-none"))
            : (document.getElementById("alertFovDiv").classList.add("d-block"),
                document.getElementById("alertFovDiv").classList.remove("d-none"));
    }),
    (createAlertFov = () => {
        var e = document.createElement("div");
        (e.id = "alertFovDiv"),
            e.classList.add("d-none"),
            (e.innerHTML =
                "Current FoV Profile is different from the one selected in Voyager");
        var t = document.querySelector(".aladin-fov");
        t.after(e);
    }),
    (chkIfMos = () => {
        let e = mosaicSettings.Wnum > 1 || mosaicSettings.Hnum > 1;
        return e;
    }),
    (moreInfoRotAdjXp = () => {
        $("#moreInfoRotBtn").html(function () {
            var e = "More info about Rotation adjust",
                t = "Close info";
            return $("#rotationAdjDesc").is(":visible") ? e : t;
        });
    }),
    (showMosListPanel = (e) => {
        e
            ? $("#csvListCont, #csvCard").removeClass("offscreen")
            : $("#csvListCont, #csvCard").addClass("offscreen");
    }),
    document
        .getElementById("moreInfoRotBtn")
        .addEventListener("click", moreInfoRotAdjXp),
    (stopConnectingStatusForAbort = () => { }),
    (invertColor = (e) => {
        if (
            (0 === e.indexOf("#") && (e = e.slice(1)),
                3 === e.length && (e = e[0] + e[0] + e[1] + e[1] + e[2] + e[2]),
                6 !== e.length)
        )
            throw new Error("Invalid HEX color.");
        var t = (255 - parseInt(e.slice(0, 2), 16)).toString(16),
            o = (255 - parseInt(e.slice(2, 4), 16)).toString(16),
            a = (255 - parseInt(e.slice(4, 6), 16)).toString(16);
        return "#" + padZeroStr(t) + padZeroStr(o) + padZeroStr(a);
    }),
    (setcolorContrast = (e) => {
        "#" === e.slice(0, 1) && (e = e.slice(1)),
            3 === e.length &&
            (e = e
                .split("")
                .map(function (e) {
                    return e + e;
                })
                .join(""));
        var t = parseInt(e.substr(0, 2), 16),
            o = parseInt(e.substr(2, 2), 16),
            a = parseInt(e.substr(4, 2), 16),
            n = (299 * t + 587 * o + 114 * a) / 1e3;
        return n >= 128 ? "black" : "white";
    }),
    (changePAbtn = (e) => {
        let t = fovdata.pAng;
        "plusPAdec" === e && (t += 0.1),
            "plusPA" === e && (t += 1),
            "minusPAdec" === e && (t -= 0.1),
            "minusPA" === e && (t -= 1),
            changePAng(t);
    });
var buttonsPA = document.querySelectorAll(".buttonPA");
if (
    ((initPAbuttons = () => {
        buttonsPA.forEach(function (e) {
            e.addEventListener("click", function () {
                changePAbtn(e.id);
            });
        });
    }),
        (isNumberKey = (e) => {
            var t = e.keyCode ? e.keyCode : e.which;
            (!((t >= 48 && t <= 57) || (t >= 96 && t <= 105) || 8 == t || 46 == t) ||
                (46 == t && -1 != $(this).val().indexOf("."))) &&
                e.preventDefault();
        }),
        void 0 === Number.prototype.toRad &&
        (Number.prototype.toRad = function () {
            return (this * Math.PI) / 180;
        }),
        void 0 === window.fovReqAnim)
) {
    for (
        var lastTime = 0,
        vendorsR = ["r", "msR", "mozR", "webkitR", "oR"],
        vendorsC = ["c", "msC", "mozC", "webkitC", "oC"],
        x = 0;
        x < vendorsR.length && void 0 === window.fovReqAnim;
        ++x
    )
        (window.fovReqAnim = window[vendorsR[x] + "equestAnimationFrame"]),
            (window.fovCanAnim =
                window[vendorsC[x] + "ancelAnimationFrame"] ||
                window[vendorsC[x] + "ancelRequestAnimationFrame"]);
    (void 0 !== window.fovReqAnim && void 0 !== window.fovCanAnim) ||
        ((window.fovReqAnim = function (e) {
            var t = new Date().getTime(),
                o = Math.max(0, 16 - (t - lastTime)),
                a = window.setTimeout(function () {
                    e(t + o);
                }, o);
            return (lastTime = t + o), a;
        }),
            (window.fovCanAnim = function (e) {
                clearTimeout(e);
            }));
}
void 0 === Number.prototype.getSign &&
    (Number.prototype.getSign = function () {
        return this > 0 ? 1 : this < 0 ? -1 : 1;
    }),
    void 0 === Number.prototype.toTrunc &&
    (Number.prototype.toTrunc = function () {
        return Math[this > 0 ? "floor" : "ceil"](this);
    });
var virtualFoV = [],
    vFoVdata = [],
    vFoVfrm = 0;
void 0 === Aladin.prototype.addCanvas &&
    (Aladin.prototype.addCanvas = function (e) {
        var t = document.createElement("canvas");
        (t.id = e),
            (t.width = aladin.getSize()[0]),
            (t.height = aladin.getSize()[1]),
            (t.style.zIndex = 2),
            (t.style.position = "absolute"),
            (t.style.left = 0),
            (t.style.top = 0),
            $("#" + this.aladinDiv.id).append(t),
            this.hasOwnProperty("ctx") || (this.ctx = []),
            (this.ctx[e] = $("#" + t.id)[0].getContext("2d")),
            this.ctx[e].translate(this.getSize()[0] / 2, this.getSize()[1] / 2);
    }),
    void 0 === Aladin.prototype.resizeCanvas &&
    (Aladin.prototype.resizeCanvas = function (e) {
        this.ctx[e].setTransform(1, 0, 0, 1, 0, 0),
            (this.ctx[e].canvas.width = this.getSize()[0]),
            (this.ctx[e].canvas.height = this.getSize()[1]),
            this.ctx[e].translate(this.getSize()[0] / 2, this.getSize()[1] / 2);
    }),
    void 0 === Aladin.prototype.removeCanvas &&
    (Aladin.prototype.removeCanvas = function (e) {
        $("#" + e).remove(), delete this.ctx[e];
    }),
    void 0 === Aladin.prototype.progressiveFov &&
    (Aladin.prototype.progressiveFov = function (e, t) {
        for (this.setFov(e); t ? this.getFov()[0] > e : this.getFov()[0] < e;)
            this.decreaseZoom(1);
    });
$("#resetSearchBtn").click(resetSearchField),
    $("#searchBtn").click({ ident: 0 }, remoteSearchTarget),
    $("#searchBtnSimbad").click({ ident: 1 }, remoteSearchTarget),
    (getProfileNameReceived = (e) => {
        updateActualProfile(e.Profile);
    }),
    (updateActualProfile = (e) => {
        (actualProfileName = sanitizeProfileName(e)),
            (connectedProfileName = actualProfileName),
            (fovdata.profileName = actualProfileName),
            setTimeout(function () {
                RemoteGetCCDSizeInfoEx();
            }, 800);
    }),
    (sanitizeProfileName = (e) => {
        let t = e.replace(/\s+/g, "_");
        return t;
    }),
    $("#preciseGoTo").click(function () {
        "" === $("#raSearch").val() && "" === $("#decSearch").val()
            ? errorFire(
                "Please insert RA and DEC J2000 coordinates or make a Search",
                "Empty coords field"
            )
            : precisePointTarget();
    }),
    $("#alaPreciseGoto").click(function () {
        (raDec = aladin.getRaDec()),
            null == raDec[0] || null == raDec[1]
                ? errorFire("Error: Please check Aladin coordinates")
                : ((raDec[0] = raDec[0] / 15), precisePointTargetNumber(raDec));
    }),
    $("#alaPreciseGotoText").click(function () {
        (raDec = aladin.getRaDec()),
            null == raDec[0] || null == raDec[1]
                ? errorFire("Error: Please check Aladin coordinates")
                : ((raDec[0] = raDec[0] / 15), precisePointTargetText(raDec));
    });
function remoteSearchTarget(e) {
    var t = { method: "RemoteSearchTarget", params: {} };
    (t.params.UID = generateUID()),
        (t.params.Name = $("#nameToSearch").val()),
        (t.params.SearchType = e.data.ident),
        pushUid("RemoteSearchTarget", t.params.UID),
        (tempUIDsearch = t.params.UID),
        doSend(JSON.stringify(t)),
        removeObjectDetail();
}
var mountButtons = document.querySelectorAll(".mount-btn");
mountButtons.forEach((e) => {
    e.addEventListener("click", (e) => {
        remoteMountFastCommand(e.target.getAttribute("ct"));
    });
}),
    (remoteMountFastCommandreceived = () => { }),
    (remoteMountFastCommandError = (e, t) => {
        errorFire(e + " " + actionResultCodes[t], "Remote action mount error");
    }),
    $("#resetLog").click(function () {
        document.getElementById("logDiv").innerHTML = "";
    });
var btnDub = document.querySelectorAll(".doubleConfirm");
btnDub.forEach((e) => {
    e.addEventListener("click", (e) => {
        doubleConfirmAction(e.currentTarget);
    });
}),
    (addFovMosaicLoaded = (e) => {
        if ((handleProfileAlert(), renderFoV(e), !sameFov)) {
            (actualProfileName = "Last_Mosaic_Loaded"),
                (fovdata.profileName = actualProfileName);
            let e = document.querySelector("#textSetupAladin");
            (e.innerHTML = "Profile FoV data selected: " + actualProfileName),
                handleLocalStorageProfile();
        }
        $("#fdb-pAng").val(fovdata.pAng),
            $("#paInd").text(" " + fovdata.pAng + "°");
    }),
    (renderFoV = (e) => {
        populateFovDataValues(e),
            saveLocalStorageAlad(),
            populateAladinfield(),
            vFoVStart(),
            null !== connectedProfileName && handleProfileAlert();
    }),
    (populateFovDataValues = (e) => {
        (actualCcdData = e),
            (fovdata.pixsize = e.PixelSize),
            (fovdata.DX = e.DX),
            (fovdata.DY = e.DY),
            (fovdata.pAng = e.PA),
            (fovdata.focal = e.Focallen),
            (fovdata.xsize = Math.round((fovdata.DX * fovdata.pixsize) / 1e3)),
            (fovdata.ysize = Math.round((fovdata.DY * fovdata.pixsize) / 1e3));
        var t = calcfov(
            fovdata.xsize,
            fovdata.ysize,
            fovdata.pixsize,
            fovdata.focal
        );
        (fovdata.fovx = parseFloat(t[0])),
            (fovdata.fovy = parseFloat(t[1])),
            (fovdata.reso = t[2]);
    }),
    $("#remoteSolveAndSync").click(function () {
        sendRemoteSolveAndSync();
    }),
    $("#remoteSolveNoSync").click(sendRemoteSolveNoSync),
    $("#remoteSolveActualPositionFov").click(sendRemoteSolveFov),
    $("#collapseResetLocalStorage").click(() => {
        $("#resetLocalStorageCont").collapse("toggle");
    }),
    $("#collapseSolvingTable").click(toggleCollapsePlSolvRes),
    (getVoyagerProfiles = (e) => {
        let t = e || !1;
        var o = { method: "RemoteGetVoyagerProfiles", params: {} };
        (o.params.UID = generateUID()),
            pushUid(t ? "RemoteGetVoyagerProfilesstartup" : o.method, o.params.UID),
            doSend(JSON.stringify(o));
    }),
    (remoteVoyagerProfileStartupReceived = (e) => {
        let t = e.list,
            o = t.find((e) => !0 === e.isactive).name;
        $("#selectedProfile").text(o), updateActualProfile(o);
    }),
    (remoteGetVoyagerProfilesReceived = (e) => {
        let t = e.list,
            o = t.find((e) => !0 === e.isactive).name;
        $("#selectedProfile").text(o),
            startupConnected ||
            (armCloseProfileList(),
                $(".prof-li").remove(),
                $("#cont-profile-list").removeClass("d-none"),
                t.length > 0
                    ? (t.forEach((e) => {
                        let t = e.isactive ? "text-primary" : "",
                            o = e.isactive ? "disabled" : "",
                            a = "active",
                            n = e.isactive ? a : "select";
                        $("#profileUl").append(
                            '<li class="list-group-item bg-dark align-items-center prof-li"><div class="row w-100"><div class="info-data-item text-left px-1 col-11 my-auto ' +
                            t +
                            '">' +
                            e.name +
                            '</div><div class="col-1 my-auto p-0"><button class="btn btn-sm btn-primary m-auto flex-end px-2 py-1 btProf ' +
                            o +
                            '" type="button" name="' +
                            e.name +
                            '">' +
                            n +
                            "</button></div></div></li>"
                        );
                    }),
                        armProfSelBtn())
                    : (errorFire(
                        "No Profile file avaiable into the default folder!",
                        "Profiles Not Found"
                    ),
                        $("#cont-profile-list").addClass("d-none")));
    }),
    (armProfSelBtn = () => {
        const e = ".btProf",
            t = document.querySelectorAll(e);
        t.forEach((e) => {
            e.addEventListener("click", (e) => {
                let t = e.currentTarget.name;
                $("#cont-profile-list").addClass("d-none"),
                    $("#selectedProfile").text(""),
                    setNewProfile(t);
            });
        });
    }),
    (armCloseProfileList = () => {
        document
            .querySelector("#closeProfileListtBtn")
            .addEventListener("click", (e) => {
                $(".prof-li").remove(), $("#cont-profile-list").addClass("d-none");
            });
    }),
    (setNewProfile = (e) => {
        var t = { method: "RemoteSetProfile", params: {} };
        (t.params.UID = generateUID()),
            (t.params.FileName = e),
            pushUid(t.method, t.params.UID),
            doSend(JSON.stringify(t));
    }),
    (remoteSetProfileReceived = () => { }),
    (profileChangedReceived = (e) => {
        $("#selectedProfile").text(e.NewProfile), updateActualProfile(e.NewProfile);
    }),
    (remoteRotatorMoveTo = (e) => {
        var t = { method: "RemoteRotatorMoveTo", params: {} };
        (t.params.UID = generateUID()),
            (t.params.PA = parseFloat(sanitizeDecimalValueStringPA(e))),
            (t.params.IsWaitAfter = !0),
            (t.params.WaitAfterSeconds = 1),
            pushUid(t.method, t.params.UID),
            doSend(JSON.stringify(t)),
            (document.querySelector("#rotatorPAinput").value = t.params.PA);
    }),
    (RemoteRotatorMoveToReceived = (e) => { }),
    (RemoteRotatorMoveToReceivedError = (e, t) => {
        errorFire(e, t);
    }),
    (sanitizeDecimalValueStringPA = (e) => (
        "string" == typeof e &&
        (e.replace(",", ".").replace(" ", ""),
            e > 360
                ? ((e = "359.99"),
                    (document.querySelector("#rotatorPAinput").value = 359.99))
                : (document.querySelector("#rotatorPAinput").value =
                    null == e || null == e || "" == e ? 0 : e)),
        e
    )),
    $("#clearFormRc").click(resetRcField),
    (showTiles = (e) => {
        let t = roboClipTemp.find((t) => t.guid === e),
            o = t.tiles,
            a = csvToArray(o);
        setClipboardMosList(o),
            a.unshift(["Tile name", "RAJ2000", "DECJ2000", "PA"]);
        let n = document.querySelector("#csvListDiv"),
            r = t.targetname;
        const i = document.querySelector("#mosListTargetName");
        (i.innerText = r), (n.textContent = "");
        let s = document.createElement("table");
        (s.classList = "mount-panel-item"),
            (s.style.width = "100%"),
            s.setAttribute("border", "1");
        let l = document.createElement("tbody");
        a.forEach(function (e) {
            let t = document.createElement("tr");
            e.forEach(function (e) {
                let o = document.createElement("td");
                (o.classList = "p-1"),
                    o.appendChild(document.createTextNode(e)),
                    t.appendChild(o);
            }),
                l.appendChild(t);
        }),
            s.appendChild(l),
            n.appendChild(s),
            showMosListPanel(!0);
    }),
    (rCgetCoordsAndPa = (e, t = !0) => {
        let o = t,
            a = roboClipTemp.find((t) => t.guid === e),
            n = a.raj2000,
            r = a.decj2000,
            i = a.pa,
            s = (a.name, JSON.parse(a.ismosaic)),
            l = a.fcol,
            c = a.frow,
            d = a.overlap,
            m = JSON.parse(a.angleadj),
            u = a.focallen,
            p = a.pixelsize,
            g = a.dx,
            v = a.dy;
        if (
            (aladin.gotoRaDec(15 * n, r),
                getCoordinateAladin(),
                $("#fdb-pAng").val(i),
                $("#paInd").text(" " + i + "°"),
                changePAng(i),
                $("#inputsearch").val(""),
                o && $("#roboClipModal").modal("hide"),
                s)
        ) {
            let e = {};
            (e.DX = g),
                (e.DY = v),
                (e.Focallen = u),
                (e.PixelSize = p),
                (e.PA = i),
                addFovMosaicLoaded(e),
                setNewValuesMosaic(c, l, d, m),
                setTimeout(vFoVStart, 500);
        } else resetMosaic();
    }),
    $("#orderDate").removeClass("btn-light").addClass("btn-warning"),
    $(".orderBtn").on("click", function (e) {
        let t = parseInt(this.getAttribute("data-param"));
        t != roboClipOrder &&
            ($("#roboClipModal").modal("hide"),
                setTimeout(function () {
                    (roboClipOrder = t), sendRemoteRoboClipGetTargetList();
                }, 500));
    }),
    $("#newFormRc").click(function () {
        resetRcField();
        let e = generateUID();
        $("#rcUid").val(e), enableSaveDelBtn(!0);
    }),
    $("#saveFormRc").click(function () {
        let e = $("#rcUid").val(),
            t = $("#raRc").val();
        if ("" != e) {
            let o = roboClipTemp.find((t) => t.guid === e);
            null != o
                ? remoteRoboClipUpdateTarget(e)
                : "" != t
                    ? remoteRoboClipAddTarget(e)
                    : showRcAlert("#rcWarning", "Please insert coords"),
                preResetFilters(),
                (roboClipOrder = 0);
        }
    }),
    $("#deleteFormRc").click(function () {
        let e = $("#rcUid").val();
        if ("" != e) {
            let t = roboClipTemp.find((t) => t.guid === e);
            if (null != t) {
                let o = t.targetname;
                modalDoubleWarnFire(
                    "Are you sure to remove target: " + o,
                    "",
                    "delFromRoboClip",
                    e
                );
            } else showRcAlert("#rcWarning", "Cannot Delete Target Before Save it.");
        } else showRcAlert("#rcWarning", "Select a Target from the list above.");
    }),
    $("#filterMenuSelId")
        .children()
        .click(function () {
            executeFilterApp(this.innerText);
        }),
    $("#clear-btn-id").click(resetFilterCont),
    $("#applyFilterBtn").click(function () {
        rcFilterActivation();
    }),
    (getMosaicCsvText = () => {
        let e = "";
        for (var t = 0; t < mosaicTiles.length; t++) {
            var o = "PANE " + (t + 1);
            (o += ";"),
                (o += mosaicTiles[t].raHours),
                (o += ";"),
                (o += mosaicTiles[t].decDegr),
                (o += ";"),
                (o += mosaicTiles[t].singlePA),
                (e += o + "\r\n");
        }
        return e;
    }),
    (csvToArray = (e) => {
        var t = [];
        return (
            e.split("\n").forEach(function (e) {
                var o = [];
                e.split(";").forEach(function (e) {
                    o.push(e);
                }),
                    "" != o && t.push(o);
            }),
            t
        );
    }),
    (createTableFromArray = (e) => {
        let t = e,
            o = "";
        return (
            t.forEach(function (e) {
                (o += "<tr>"),
                    e.forEach(function (e) {
                        o += "<td>" + e + "</td>";
                    }),
                    (o += "</tr>");
            }),
            o
        );
    }),
    (r2d = (e) => {
        let t = Math.PI;
        return e * (180 / t);
    }),
    (d2r = (e) => {
        let t = Math.PI;
        return e * (t / 180);
    }),
    (calculateSingleRot = (e) => {
        let t,
            o = {};
        o.y = e.y - 0.1;
        let a = aladin.pix2world(e.x, e.y),
            n = aladin.pix2world(e.x, o.y);
        (a[0] = d2r(a[0])),
            (a[1] = d2r(a[1])),
            (n[0] = d2r(n[0])),
            (n[1] = d2r(n[1]));
        let r = n[0] - a[0];
        return (
            (t = Math.atan2(
                Math.sin(r),
                Math.cos(a[1]) * Math.tan(n[1]) - Math.sin(a[1]) * Math.cos(r)
            )),
            (t = r2d(t)),
            t
        );
    });
(Array.prototype.max = function () {
    return Math.max.apply(null, this);
}),
    (Array.prototype.min = function () {
        return Math.min.apply(null, this);
    });

// ----------------------------------------------------------------
// Function written by Max Qian
// ----------------------------------------------------------------

document.getElementById("fov_setting_btn").addEventListener("click", fov_setting_click)
function fov_setting_click(){
    let sensor_pixel_size = $('#sensor_pixel_size').val(),
        sensor_width = $('#sensor_width').val(),
        sensor_height = $('#sensor_height').val(),
        focal_length = $('#focal_length').val()
    if(sensor_pixel_size <= 0 || sensor_height <= 0 || sensor_width <= 0 || focal_length <= 0) {
        errorFire("Invalid value of sensor or focal length","Invalid value")
    }
    console.log("Sensor pixel size: " + sensor_pixel_size)
    console.log("Sensor width: " + sensor_width)
    console.log("Sensor height: " + sensor_height)
    console.log("Focal length: " + focal_length)
    let fov = calcfov(sensor_width, sensor_height, sensor_pixel_size,focal_length)
    console.log("FOV: " + fov)
    $("#setupReso").text(fov[2])
    $("#setupFocal").text(focal_length)
    $("#fovXsize").text(fov[0])
    $("#fovYsize").text(fov[1])
    $("#chipXsizePx").text("-")
    $("#chipYsizePx").text("-")
    $("#pixSize").text(sensor_pixel_size)
    $("#chipXsize").text(sensor_width)
    $("#chipYsize").text(sensor_height)
}