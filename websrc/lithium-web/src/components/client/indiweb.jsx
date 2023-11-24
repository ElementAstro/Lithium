import React, { useState, useEffect } from 'react';
import $ from 'jquery';

function INDIWebComponent() {

    const [selectedProfile, setSelectedProfile] = useState('');
    const [remoteDrivers, setRemoteDrivers] = useState('');
    const [profileInfo, setProfileInfo] = useState({
        port: '',
        autostart: false,
        autoconnect: false
    });

    useEffect(() => {
        $('[data-toggle="tooltip"]').tooltip();
        loadCurrentProfileDrivers();
        getStatus();
    }, []);

    const saveProfile = () => {
        const name = selectedProfile.trim();
        const url = `/indiweb/api/profiles/${name}`;

        console.log(url);

        $.ajax({
            type: 'POST',
            url: encodeURI(url),
            success: () => {
                console.log(`Add new a profile ${name}`);
                saveProfileDrivers(name);
            },
            error: () => {
                $("#errorMessage").html("添加配置文件出错"),
                    $("#centralModalDanger").modal("show");
            }
        });
    };

    const saveProfileInfo = () => {
        const name = selectedProfile.trim();
        const { port, autostart, autoconnect } = profileInfo;
        const url = `/indiweb/api/profiles/${name}`;

        const profileData = {
            port,
            autostart: autostart ? 1 : 0,
            autoconnect: autoconnect ? 1 : 0
        };

        const profileInfoJson = JSON.stringify(profileData);

        console.log(`Profile info ${profileInfoJson}`);
        console.log(url);

        $.ajax({
            type: 'PUT',
            url: encodeURI(url),
            data: profileInfoJson,
            contentType: "application/json; charset=utf-8",
            success: () => {
                console.log(`Profile ${name} info is updated`);
            },
            error: () => {
                $("#errorMessage").html("更新配置出错"),
                    $("#centralModalDanger").modal("show");
            }
        });
    };

    const saveProfileDrivers = (profile, silent) => {
        if (typeof (silent) === 'undefined') silent = false;

        var url = "/indiweb/api/profiles/" + profile + "/drivers";
        var drivers = [];

        $("#drivers_list :selected").each(function (i, sel) {
            drivers.push({
                "label": $(sel).text()
            });
        });

        // Check for remote drivers
        var remote = $("#remote_drivers").val();
        if (remote) {
            drivers.push({
                "remote": remote
            });
            console.log({
                "remote": remote
            });
        }

        drivers = JSON.stringify(drivers);

        //console.log("my json string is " + drivers);
        console.log(drivers);
        $.ajax({
            type: 'POST',
            url: encodeURI(url),
            data: drivers,
            contentType: "application/json; charset=utf-8",
            success: function () {
                console.log("Drivers added successfully to profile");
                if (silent === false)
                    $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>配置 ' + profile + ' 已保存</div>');
            },
            error: function () {
                $("#errorMessage").html("无法添加设备"),
                    $("#centralModalDanger").modal("show");
            }
        });
    };

    const loadCurrentProfileDrivers = () => {
        clearDriverSelection();

        var name = $("#profiles option:selected").text();
        var url = "/indiweb/api/profiles/" + name + "/labels";

        $.getJSON(url, function (drivers) {
            $.each(drivers, function (i, driver) {
                var label = driver.label;
                //console.log("Driver label is " + label);
                var selector = "#drivers_list [value='" + label + "']";
                $(selector).prop('selected', true);
            });

            $("#drivers_list").selectpicker('refresh');
        });

        url = encodeURI("/indiweb/api/profiles/" + name + "/remote");

        $.getJSON(url, function (data) {
            if (data && data.drivers !== undefined) {
                $("#remote_drivers").val(data.drivers);
            }
            else {
                $("#remote_drivers").val("");
            }
        });

        loadProfileData();

    };

    const loadProfileData = () => {
        var name = $("#profiles option:selected").text();
        var url = encodeURI("/indiweb/api/profiles/" + name);

        $.getJSON(url, function (info) {
            if (info.autostart == 1)
                $("#profile_auto_start").prop("checked", true);
            else
                $("#profile_auto_start").prop("checked", false);

            if (info.autoconnect == 1)
                $("#profile_auto_connect").prop("checked", true);
            else
                $("#profile_auto_connect").prop("checked", false);

            $("#profile_port").val(info.port);

        });
    };

    const clearDriverSelection = () => {
        $("#drivers_list option").prop('selected', false);
        $("#drivers_list").selectpicker('refresh');
        // Uncheck Auto Start
        $("#profile_auto").prop("checked", false);
        $("#profile_port").val("7624");
    };

    const addNewProfile = () => {
        var profile_name = $("#new_profile_name").val();
        if (profile_name) {
            //console.log("profile is " + profile_name);
            $("#profiles").append("<option id='" + profile_name + "' selected>" + profile_name + "</option>");

            clearDriverSelection();

            $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>配置 ' + profile_name + '已创建,请选择设备并保存</div>');
        }
    };

    const removeProfile = () => {
        //console.log("in delete profile");
        var name = $("#profiles option:selected").text();
        var url = "/indiweb/api/profiles/" + name;

        console.log(url);

        if ($("#profiles option").size() == 1) {
            $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>无法删除默认配置</div>');
            return;
        }

        $.ajax({
            type: 'DELETE',
            url: encodeURI(url),
            success: function () {
                console.log("delete profile " + name);
                $("#profiles option:selected").remove();
                $("#profiles").selectpicker('refresh');
                loadCurrentProfileDrivers();

                $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>配置 ' + name + ' 已删除</div>');
            },
            error: function () {
                $("#errorMessage").html("无法删除配置！"),
                    $("#centralModalDanger").modal("show");
            }
        });
    }

    const toggleServer = () => {
        var status = $.trim($("#server_command").text());

        if (status == "启动") {
            var profile = $("#profiles option:selected").text();
            var url = "/indiweb/api/server/start/" + profile;

            $.ajax({
                type: 'POST',
                url: encodeURI(url),
                success: function () {
                    //console.log("INDI Server started!");
                    getStatus();
                },
                error: function () {
                    $("#errorMessage").html("无法启动INDI服务器"),
                        $("#centralModalDanger").modal("show");
                }
            });
        } else {
            $.ajax({
                type: 'POST',
                url: "/indiweb/api/server/stop",
                success: function () {
                    //console.log("INDI Server stopped!");
                    getStatus();
                },
                error: function () {
                    $("#errorMessage").html("无法停止INDI服务器"),
                        $("#centralModalDanger").modal("show");
                }
            });
        }
    }

    const getStatus = () => {
        $.getJSON("/indiweb/api/server/status", function (data) {
            if (data[0].status == "True")
                getActiveDrivers();
            else {
                $("#server_command").html("<i class='fas fa-light-switch-on'></i> 启动");
                $("#server_notify").html("<p class='alert alert-warning'>服务器抽风了！</p>");
            }

        });
    }

    const getActiveDrivers = () => {
        $.getJSON("/indiweb/api/server/drivers", function (data) {
            $("#server_command").html("<i class='fas fa-close'></i> 停止");
            var msg = "<p class='alert alert-info'>服务器在线啦！<ul  class=\"list-unstyled\">";
            var counter = 0;
            $.each(data, function (i, field) {
                msg += "<li class='border border-primary mb-1 stupid-text'>" + "<button class=\"btn btn-xs\" " +
                    "onCLick=\"restartDriver('" + field.label + "')\" data-toggle=\"tooltip\" " +
                    "title=\"重启设备\">" +
                    "<i class='fas fa-redo'></i></button> " +
                    field.label + "</li>";
                counter++;
            });

            msg += "</ul></p>";

            $("#server_notify").html(msg);

            if (counter < $("#drivers_list :selected").length) {
                $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>不是所有设备都在正常工作,请检查电力连接</div>');
                return;
            }
        });

    }

    const restartDriver = (label) => {
        var url = "/indiweb/api/drivers/restart/" + label;
        $.ajax({
            type: 'POST',
            url: encodeURI(url),
            success: function () {
                getStatus();
                $("#notify_message").html('<br/><div class="alert alert-success"><a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>设备 "' + label + '" 重启成功</div>');
            },
            error: function () {
                $("#errorMessage").html("重启设备失败"),
                    $("#centralModalDanger").modal("show");
            }
        });
    }

    return (
        <div>
            {/* JSX内容 */}
        </div>
    );
}

export default INDIWebComponent;
