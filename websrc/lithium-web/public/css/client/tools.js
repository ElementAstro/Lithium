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
}

$(document).ready(function () {
  $("#get_time").on("click", getTime);
  $("#sync_time").on("click", syncTime);
  $("#reset_time").on("click", resetTime);
});

function getTime() {
  const now = new Date();
  const year = now.getFullYear();
  const month = (now.getMonth() + 1).toString().padStart(2, "0");
  const day = now.getDate().toString().padStart(2, "0");
  const hour = now.getHours().toString().padStart(2, "0");
  const minute = now.getMinutes().toString().padStart(2, "0");
  const second = now.getSeconds().toString().padStart(2, "0");
  const time = `${year}:${month}:${day}-${hour}:${minute}:${second}`;
  console.log(time);
  $("#time").val(time);
  return time;
}

function syncTime() {
  $.ajax({
    url: `/tools/api/time/${getTime()}`,
    type: "GET",
  })
    .done(function (data) {
      if (!data.error) {
        console.log(data.message);
        $("#time_info_message").text("Info: " + data.message);
        $("#time_info").removeAttr("hidden");
      } else {
        onError(data.error);
        $("#time_error_message").text("Error: " + data.error);
        $("#time_error").removeAttr("hidden");
      }
    })
    .fail(function () {
      onError("Failed to send request to server");
    });
}

function resetTime() {
  $("#time").val("");
}

// ----------------------------------------------------------------
// Get location via client gps
// ----------------------------------------------------------------

$(document).ready(function () {
  $("#get_location").on("click", getLocation);
  $("#sync_location").on("click", syncLocation);
  $("#reset_location").on("click", resetLocation);
});

function getLocation() {
  let options = {
    enableHighAccuracy: true,
    maximumAge: 1000,
  };

  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(
      onLocationSuccess,
      onLocationError,
      options
    );
  } else {
    onError("Web browser does not support GPS function");
  }
}

function syncLocation() {
  $.ajax({
    url: "/tools/api/location/" + $("#lon").val() + "/" + $("#lat").val(),
    type: "GET",
  })
    .done(function (data) {
      if (!data.error) {
        console.log(data.message);
        $("#location_info_message").text("Info: " + data.message);
        $("#location_info").removeAttr("hidden");
      } else {
        onError(data.error);
        $("#location_error_message").text("Error: " + data.error);
        $("#location_error").removeAttr("hidden");
      }
    })
    .fail(function () {
      onError("Failed to send request to server");
    });
}

function resetLocation() {
  $("#lon").val("");
  $("#lat").val("");
}

function onLocationSuccess(pos) {
  console.log("Lon:" + pos.coords.longitude + "Lat:" + pos.coords.latitude);
  $("#lon").val(pos.coords.longitude);
  $("#lat").val(pos.coords.latitude);
}

function onLocationError(error) {
  const errorMessages = {
    1: "Positioning function rejected",
    2: "Unable to obtain location information temporarily",
    3: "Get information timeout",
    4: "Unknown error",
  };

  onError(errorMessages[error.code]);
}

// ----------------------------------------------------------------
// Update the sotfwares
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// Download the template file for astromical solver
// ----------------------------------------------------------------

$(() => {
  $("#solver").change(() => {
    console.log($("#solver").val());
    let solver = $("#solver").val();
    // If want to downlaad astrometry templates
    if (solver == "astrometry") {
      // First check whether the files had already downloaded
      $.ajax({
        url: "/tools/api/download/astrometry/already",
        type: "GET",
        async: true,
        processData: false,
        contentType: "application/json",
        success: function (data) {
          if (!data.error) {
            console.log(data.message);
          } else {
            onError(data.error);
          }
        },
        error: function () {
          onError("Failed to send request to server");
        },
      });
    } else {
      // Same step as astrometry
      $.ajax({
        url: "/tools/api/download/astap/already",
        type: "GET",
        async: true,
        processData: false,
        contentType: "application/json",
        success: function (data) {
          if (!data.error) {
            console.log(data.message);
          } else {
            onError(data.error);
          }
        },
        error: function () {
          onError("Failed to send request to server");
        },
      });
    }
  });
});

function LoadStarInfomation() {
  $.ajax({
    url: "/tools/api/starinfo",
    type: "GET",
    async: true,
    processData: false,
    contentType: "application/json",
    success: function (data) {
      if (!data.error) {
        $("#polaris_next_transit").html(data.polaris_next_transit);
        $("#polaris_alt").html(data.polaris_alt);
        $("#moon_phase").html(data.moon_phase + " (" + data.moon_light + "%)");
        $("#moon_rise").html(data.moon_rise);
        $("#moon_transit").html(data.moon_transit);
        $("#moon_set").html(data.moon_set);
        $("#moon_az").html(data.moon_az);
        $("#moon_alt").html(data.moon_alt);
        $("#moon_ra").html(data.moon_ra);
        $("#moon_dec").html(data.moon_dec);
        $(".moon_new").html(data.moon_new);
        $("#moon_full").html(data.moon_full);
        $("#sun_at_start").html(data.sun_at_start);
        $("#sun_ct_start").html(data.sun_ct_start);
        $("#sun_rise").html(data.sun_rise);
        $("#sun_transit").html(data.sun_transit);
        $("#sun_set").html(data.sun_set);
        $("#sun_ct_end").html(data.sun_ct_end);
        $("#sun_at_end").html(data.sun_at_end);
        $("#sun_az").html(data.sun_az);
        $("#sun_alt").html(data.sun_alt);
        $("#sun_ra").html(data.sun_ra);
        $("#sun_dec").html(data.sun_dec);
        $("#sun_equinox").html(data.sun_equinox);
        $(".sun_solstice").html(data.sun_solstice);
        $("#mercury_rise").html(data.mercury_rise);
        $("#mercury_transit").html(data.mercury_transit);
        $("#mercury_set").html(data.mercury_set);
        $("#mercury_az").html(data.mercury_az);
        $("#mercury_alt").html(data.mercury_alt);
        $("#venus_rise").html(data.venus_rise);
        $("#venus_transit").html(data.venus_transit);
        $("#venus_set").html(data.venus_set);
        $("#venus_az").html(data.venus_az);
        $("#venus_alt").html(data.venus_alt);
        $("#mars_rise").html(data.mars_rise);
        $("#mars_transit").html(data.mars_transit);
        $("#mars_set").html(data.mars_set);
        $("#mars_az").html(data.mars_az);
        $("#mars_alt").html(data.mars_alt);
        $("#jupiter_rise").html(data.jupiter_rise);
        $("#jupiter_transit").html(data.jupiter_transit);
        $("#jupiter_set").html(data.jupiter_set);
        $("#jupiter_az").html(data.jupiter_az);
        $("#jupiter_alt").html(data.jupiter_alt);
        $("#saturn_rise").html(data.saturn_rise);
        $("#saturn_transit").html(data.saturn_transit);
        $("#saturn_set").html(data.saturn_set);
        $("#saturn_az").html(data.saturn_az);
        $("#saturn_alt").html(data.saturn_alt);
        $("#uranus_rise").html(data.uranus_rise);
        $("#uranus_transit").html(data.uranus_transit);
        $("#uranus_set").html(data.uranus_set);
        $("#uranus_az").html(data.uranus_az);
        $("#uranus_alt").html(data.uranus_alt);
        $("#neptune_rise").html(data.neptune_rise);
        $("#neptune_transit").html(data.neptune_transit);
        $("#neptune_set").html(data.neptune_set);
        $("#neptune_az").html(data.neptune_az);
        $("#neptune_alt").html(data.neptune_alt);

        var pha = data.polaris_hour_angle;
        pha_angle = 360 + pha * -1;
        pha_angle -= 180;
        var rotation = "rotate(" + pha_angle + "deg)";
        $("#polaris_marker").css("-ms-transform", rotation);
        $("#polaris_marker").css("-webkit-transform", rotation);
        $("#polaris_marker").css("transform", rotation);

        var pha = data.polaris_hour_angle;
        var phaH = String(parseInt(pha / 15));
        var phaMtmp = (pha / 15 - phaH) * 60;
        var phaM = String(parseInt(phaMtmp));
        var phaS = String(parseInt((phaMtmp - phaM) * 60));
        $("#pha").html(
          phaH.padStart(2, "0") +
            ":" +
            phaM.padStart(2, "0") +
            ":" +
            phaS.padStart(2, "0")
        );

        if (parseFloat(data.mercury_alt) > 25) {
          $("#mercury").css("color", "#99cc00");
        } else if (parseFloat(data.mercury_alt) > 0) {
          $("#mercury").css("color", "#ff9900");
        } else {
          $("#mercury").css("color", "#fff");
        }

        if (parseFloat(data.venus_alt) > 25) {
          $("#venus").css("color", "#99cc00");
        } else if (parseFloat(data.venus_alt) > 0) {
          $("#venus").css("color", "#ff9900");
        } else {
          $("#venus").css("color", "#fff");
        }

        if (parseFloat(data.mars_alt) > 25) {
          $("#mars").css("color", "#99cc00");
        } else if (parseFloat(data.mars_alt) > 0) {
          $("#mars").css("color", "#ff9900");
        } else {
          $("#mars").css("color", "#fff");
        }

        if (parseFloat(data.jupiter_alt) > 25) {
          $("#jupiter").css("color", "#99cc00");
        } else if (parseFloat(data.jupiter_alt) > 0) {
          $("#jupiter").css("color", "#ff9900");
        } else {
          $("#jupiter").css("color", "#fff");
        }

        if (parseFloat(data.saturn_alt) > 25) {
          $("#saturn").css("color", "#99cc00");
        } else if (parseFloat(data.saturn_alt) > 0) {
          $("#saturn").css("color", "#ff9900");
        } else {
          $("#saturn").css("color", "#fff");
        }

        if (parseFloat(data.uranus_alt) > 25) {
          $("#uranus").css("color", "#99cc00");
        } else if (parseFloat(data.uranus_alt) > 0) {
          $("#uranus").css("color", "#ff9900");
        } else {
          $("#uranus").css("color", "#fff");
        }

        if (parseFloat(data.neptune_alt) > 25) {
          $("#neptune").css("color", "#99cc00");
        } else if (parseFloat(data.neptune_alt) > 0) {
          $("#neptune").css("color", "#ff9900");
        } else {
          $("#neptune").css("color", "#fff");
        }

        var nm = new Date(data.moon_new);
        var fm = new Date(data.moon_full);
        if (nm < fm) {
          $("#new_moon_first").css("display", "");
          $("#new_moon_second").css("display", "none");
        } else {
          $("#new_moon_first").css("display", "none");
          $("#new_moon_second").css("display", "");
        }
      } else {
        onError(data.error);
      }
    },
    error: function () {
      onError("Failed to send request to server");
    },
  });
}
LoadStarInfomation();
