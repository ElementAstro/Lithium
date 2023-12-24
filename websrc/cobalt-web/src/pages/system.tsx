import React, { useEffect, useState } from "react";
import { Card, Button } from "react-bootstrap";
import Chart from "chart.js/auto";
import $ from "jquery";

function RealtimeDataCharts() {
  const [realtimeCpuChart, setRealtimeCpuChart] = useState(null);
  const [realtimeMemoryChart, setRealtimeMemoryChart] = useState(null);

  useEffect(() => {
    // CPU chart
    const ctxCpu = document.getElementById("realtime_cpu");
    const cpuChart = new Chart(ctxCpu, {
      type: "line",
      data: {
        datasets: [
          {
            label: "CPU用量 %",
            backgroundColor: ["rgba(76, 209, 55, 0.1)"],
            borderColor: "blue",
            borderWidth: 1,
            data: [],
          },
        ],
        labels: [],
      },
      options: {
        title: {
          display: false,
        },
        scales: {
          y: {
            beginAtZero: true,
            min: 0,
            max: 100,
          },
          x: {
            display: true,
          },
        },
      },
    });
    setRealtimeCpuChart(cpuChart);

    // Memory chart
    const ctxMemory = document.getElementById("realtime_memory");
    const memoryChart = new Chart(ctxMemory, {
      type: "line",
      data: {
        datasets: [
          {
            label: "内存用量 %",
            backgroundColor: ["rgba(76, 209, 55, 0.2)"],
            borderColor: "lightgreen",
            borderWidth: 1,
            data: [],
          },
        ],
        labels: [],
      },
      options: {
        title: {
          display: false,
        },
        scales: {
          y: {
            beginAtZero: true,
            min: 0,
            max: 100,
          },
          x: {
            display: true,
          },
        },
      },
    });
    setRealtimeMemoryChart(memoryChart);

    const memoryUpdate = setInterval(getMemoryData, 5000);
    const cpuUpdate = setInterval(getCpuData, 5000);

    return () => {
      clearInterval(memoryUpdate);
      clearInterval(cpuUpdate);
    };
  }, []);

  function formatDate(time) {
    const date = new Date(time);
    const hour = date.getHours();
    const min = date.getMinutes();
    const sec = date.getSeconds();
    const newTime = hour + ":" + min + ":" + sec;
    return newTime;
  }

  function addCpuData(data) {
    if (realtimeCpuChart.data.labels.length > 10) {
      realtimeCpuChart.data.labels.shift();
    }
    realtimeCpuChart.data.labels.push(formatDate(new Date().getTime()));
    if (data === 0) {
      realtimeCpuChart.data.datasets[0].data.push(
        realtimeCpuChart.data.datasets[0].data[
          realtimeCpuChart.data.datasets[0].data.length - 1
        ]
      );
    } else {
      if (realtimeCpuChart.data.datasets[0].data.length > 10) {
        realtimeCpuChart.data.datasets[0].data.shift();
      }
      realtimeCpuChart.data.datasets[0].data.push(data);
    }
    realtimeCpuChart.update();
  }

  function addMemoryData(data) {
    if (realtimeMemoryChart.data.labels.length > 10) {
      realtimeMemoryChart.data.labels.shift();
    }
    realtimeMemoryChart.data.labels.push(formatDate(new Date().getTime()));
    if (data === 0) {
      realtimeMemoryChart.data.datasets[0].data.push(
        realtimeMemoryChart.data.datasets[0].data[
          realtimeMemoryChart.data.datasets[0].data.length - 1
        ]
      );
    } else {
      if (realtimeMemoryChart.data.datasets[0].data.length > 10) {
        realtimeMemoryChart.data.datasets[0].data.shift();
      }
      realtimeMemoryChart.data.datasets[0].data.push(data);
    }
    realtimeMemoryChart.update();
  }

  function getMemoryData() {
    $.ajax({
      url: "/system/api/memory",
      type: "GET",
      async: true,
      processData: false,
      contentType: "application/json",
      success: function (data) {
        addMemoryData(data.used);
      },
      error: function () {
        clearInterval(memory_update);
        $("#errorMessage").text("无法获取内存数据");
        $("#centralModalDanger").modal("show");
      },
    });
  }

  function getCpuData() {
    $.ajax({
      url: "/system/api/cpu",
      type: "GET",
      async: true,
      processData: false,
      contentType: "application/json",
      success: function (data) {
        addCpuData(data.used);
      },
      error: function () {
        clearInterval(cpu_update);
        $("#errorMessage").text("无法获取CPU数据");
        $("#centralModalDanger").modal("show");
      },
    });
  }

  return (
    <div>
      <Card>
        <Card.Body>
          <canvas id="realtime_cpu" />
        </Card.Body>
        <Card.Footer>
          <Button id="refresh_cpu">刷新CPU</Button>
        </Card.Footer>
      </Card>

      <Card>
        <Card.Body>
          <canvas id="realtime_memory" />
        </Card.Body>
        <Card.Footer>
          <Button id="refresh_memory">刷新内存</Button>
        </Card.Footer>
      </Card>
    </div>
  );
}

export default RealtimeDataCharts;
