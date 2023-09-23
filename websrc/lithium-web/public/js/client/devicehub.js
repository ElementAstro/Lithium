/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

function onError(message) {
    $("#errorMessage").text(message);
    $("#centralModalDanger").modal("show");
};

// ----------------------------------------------------------------
// Provide some additional device hub functions
// ----------------------------------------------------------------

$(function () {
    getStatus();
});

document.getElementById("server_command").addEventListener("click", toggle_server)

function toggle_server(){
    var status = $.trim($("#server_command").text());
    if (status == "启动" || status == "start" || status == "Start") {

        $.ajax({
            type: 'POST',
            url: "/devicehub/api/start",
            contentType: "application/json; charset=utf-8",
            data : JSON.stringify({
                camera : $("#camera").val(),
                telescope : $("#telescope").val(),
                focuser : $("#focuser").val(),
                filterwheel : $("#filterwheel").val(),
                solver : $("solver").val(),
                guider : $("#guider").val(),
                plugins : $("#plugins").val()
            }),
            success: function () {
                getStatus();
            },
            error: function () {
            }
        });
    } else {
        $.ajax({
            type: 'GET',
            url: "/devicehub/api/stop",
            success: function () {
                getStatus();
            },
            error: function () {
            }
        });
    }
}

function getStatus() {
    $.getJSON("/devicehub/api/status", function (data) {
        if (data.status == "True")
            getActiveDrivers();
        else {
            $("#server_command").html("<i class='fas fa-light-switch-on'></i> 启动");
            $("#server_notify").html("<p class='alert alert-info'>服务器空闲中</p>");
            
            $("#camera_name").text("");
            $("#camera_status").text("");
            $('#telescope_name').text("");
            $('#telescope_status').text("");
            $('#focuser_name').text("");
            $('#focuser_status').text("");
            $('#guider_name').text("");
            $('#guider_status').text("");
            $('#solver_name').text("");
            $('#solver_status').text("");
            $('#filterwheel_name').text("");
            $('#filterwheel_status').text("");

            $("#server_command").removeClass('btn-outlinedanger');
            $("#server_command").addClass('btn-outline-primary');
        }
    });
}

function getActiveDrivers() {
    $.getJSON("/devicehub/api/drivers", function (data) {
        $("#server_command").html("<i class='fas fa-close'></i> 停止");
        $("#server_command").addClass('btn-outlinedanger');
        $("#server_command").removeClass('btn-outline-primary');

        var msg = "<p class='alert alert-info'>服务器在线啦！</p>";
        $("#server_notify").html(msg);

        if(data.camera != null) {
            $('#camera_name').text(data.camera.name);
            $('#camera_status').text("Running");
            $('#camera_restart').removeClass('disabled');
            $('#camera_disconnect').removeClass('disabled');
        }
        if(data.telescope != null){
            $('#telescope_name').text(data.telescope.name);
            $('#telescope_status').text("Running");
            $('#mount_restart').removeClass('disabled');
            $('#mount_disconnect').removeClass('disabled');
        }
        if(data.focuser != null){
            $('#focuser_name').text(data.focuser.name);
            $('#focuser_status').text("Running");
            $('#focuser_restart').removeClass('disabled');
            $('#focuser_disconnect').removeClass('disabled');
        }
        if(data.filterwheel != null){
            $('#filterwheel_name').text(data.filterwheel.name);
            $('#filterwheel_status').text("Running");
            $('#filterwheel_restart').removeClass('disabled');
            $('#filterwheel_disconnect').removeClass('disabled');
        }
        if(data.plugins != null && data.plugins.length > 0){
            
        }
    })

}

document.getElementById("camera_restart").addEventListener("click", camera_restart)
document.getElementById("camera_disconnect").addEventListener("click", camera_disconnect)

document.getElementById("mount_restart").addEventListener("click", mount_restart)
document.getElementById("mount_disconnect").addEventListener("click", mount_disconnect)

document.getElementById("focuser_restart").addEventListener("click",focuser_restart)
document.getElementById("focuser_disconnect").addEventListener("click",focuser_disconnect)

document.getElementById("filterwheel_restart").addEventListener("click",filterwheel_restart)
document.getElementById("filterwheel_disconnect").addEventListener("click",filterwheel_disconnect)

document.getElementById("solver_restart").addEventListener("click",solver_restart)
document.getElementById("solver_disconnect").addEventListener("click",solver_disconnect)

document.getElementById("guider_restart").addEventListener("click",guider_restart)
document.getElementById("guider_disconnect").addEventListener("click",guider_disconnect)

function camera_restart(){

}

function camera_disconnect(){

}

function mount_restart(){

}

function mount_disconnect(){

}

function focuser_restart(){

}

function focuser_disconnect(){

}

function filterwheel_restart(){

}

function filterwheel_disconnect(){

}

function guider_restart(){

}

function guider_disconnect(){

}

function solver_restart(){

}

function solver_disconnect(){

}