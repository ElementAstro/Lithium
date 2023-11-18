import React, { useState, useEffect } from 'react';
import $ from 'jquery';

function ToolsComponent() {
    const [time, setTime] = useState('');
    const [lon, setLon] = useState('');
    const [lat, setLat] = useState('');

    function onError(message) {
        $("#errorMessage").text(message);
        $("#centralModalDanger").modal("show");
    }

    function getTime() {
        let now = new Date();
        let year = now.getFullYear();
        let month = now.getMonth() + 1;
        let day = now.getDate();
        let hour = now.getHours();
        let minute = now.getMinutes();
        let second = now.getSeconds();
        let time = `${year}:${month}:${day}-${hour}:${minute}:${second}`;
        console.log(time);
        setTime(time);
        return time;
    }

    function syncTime() {
        $.ajax({
            url: '/tools/api/time/' + getTime(),
            type: 'GET',
            async: true,
            processData: false,
            contentType: "application/json",
            success: function (data) {
                if (!data.error) {
                    console.log(data.message);
                    $("#time_info_message").text("Info: " + data.message);
                    $("#time_info").removeAttr("hidden");
                } else {
                    onError(data.error);
                    $("#time_error_message").text("Error: " + data.error);
                    $("#time_error").removeAttr("hidden");
                }
            },
            error: function () {
                onError("Failed to send request to server");
            }
        });
    }

    function resetTime() {
        setTime('');
    }

    function getLocation() {
        let options = {
            enableHighAccuracy: true,
            maximumAge: 1000
        };
        if (navigator.geolocation) {
            navigator.geolocation.getCurrentPosition(onLocationSuccess, onLocationError, options);
        } else {
            onError("Web browser does not support GPS function");
        }
    }

    function syncLocation() {
        $.ajax({
            url: '/tools/api/location/' + lon + '/' + lat,
            type: 'GET',
            async: true,
            processData: false,
            contentType: "application/json",
            success: function (data) {
                if (!data.error) {
                    console.log(data.message);
                    $("#location_info_message").text("Info: " + data.message);
                    $("#location_info").removeAttr("hidden");
                } else {
                    onError(data.error);
                    $("#location_error_message").text("Error: " + data.error);
                    $("#location_error").removeAttr("hidden");
                }
            },
            error: function () {
                onError("Failed to send request to server");
            }
        });
    }

    function resetLocation() {
        setLon('');
        setLat('');
    }

    function onLocationSuccess(pos) {
        console.log("Lon:" + pos.coords.longitude + "Lat:" + pos.coords.latitude);
        setLon(pos.coords.longitude);
        setLat(pos.coords.latitude);
    }

    function onLocationError(error) {
        switch (error.code) {
            case 1:
                onError("Positioning function rejected");
                break;
            case 2:
                onError("Unable to obtain location information temporarily");
                break;
            case 3:
                onError("Get information timeout");
                break;
            case 4:
                onError("Unknown error");
                break;
        }
    }

    useEffect(() => {
        document.getElementById("get_time").addEventListener("click", getTime);
        document.getElementById("sync_time").addEventListener("click", syncTime);
        document.getElementById("reset_time").addEventListener("click", resetTime);
        document.getElementById('get_location').addEventListener('click', getLocation);
        document.getElementById('sync_location').addEventListener('click', syncLocation);
        document.getElementById('reset_location').addEventListener('click', resetLocation);
    }, []);

    return (
        <div>
            {/* JSX内容 */}
        </div>
    );
}

export default ToolsComponent;
