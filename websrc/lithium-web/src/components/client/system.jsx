import React, { useEffect } from 'react';
import Chart from 'chart.js/auto';
import $ from 'jquery'; // Assuming you have jQuery available

function RealtimeDataCharts() {
    let realtimeCpuChart;
    let realtimeMemoryChart;

    useEffect(() => {
        const ctxCpu = document.getElementById('realtime_cpu');
        realtimeCpuChart = new Chart(ctxCpu, {
            type: 'line',
            data: {
                datasets: [{
                    label: 'CPU用量 %',
                    backgroundColor: ['rgba(76, 209, 55, 0.1)'],
                    borderColor: 'blue',
                    borderWidth: 1,
                    data: []
                }],
                labels: []
            },
            options: {
                title: {
                    display: false
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        min: 0,
                        max: 100
                    },
                    x: {
                        display: true
                    }
                }
            }
        });

        const ctxMemory = document.getElementById('realtime_memory');
        realtimeMemoryChart = new Chart(ctxMemory, {
            type: 'line',
            data: {
                datasets: [{
                    label: '内存用量 %',
                    backgroundColor: ['rgba(76, 209, 55, 0.2)'],
                    borderColor: 'lightgreen',
                    borderWidth: 1,
                    data: []
                }],
                labels: []
            },
            options: {
                title: {
                    display: false
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        min: 0,
                        max: 100
                    },
                    x: {
                        display: true
                    }
                }
            }
        });

        const memoryUpdate = setInterval(getMemoryData, 5000);
        const cpuUpdate = setInterval(getCpuData, 5000);

        return () => {
            clearInterval(memoryUpdate);
            clearInterval(cpuUpdate);
        };
    }, []);

    function formatDate(time) {
        var date = new Date(time);
        var hour = date.getHours(),
            min = date.getMinutes(),
            sec = date.getSeconds();
        var newTime = hour + ':' +
            min + ':' +
            sec;
        return newTime;
    }

    function addCpuData(data) {
        if (realtimeCpuChart.data.labels.length > 10) {
            realtimeCpuChart.data.labels.shift();
        }
        realtimeCpuChart.data.labels.push(formatDate(new Date().getTime()));
        if (data === 0) {
            realtimeCpuChart.data.datasets[0].data.push(realtimeCpuChart.data.datasets[0].data[realtimeCpuChart.data.datasets[0].data.length - 1]);
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
            realtimeMemoryChart.data.datasets[0].data.push(realtimeMemoryChart.data.datasets[0].data[realtimeMemoryChart.data.datasets[0].data.length - 1]);
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
            url: '/system/api/memory',
            type: 'GET',
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
            }
        })
    }

    function getCpuData() {
        $.ajax({
            url: '/system/api/cpu',
            type: 'GET',
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
            }
        })
    }

    return (
        <div>
            <canvas id="realtime_cpu" />
            <canvas id="realtime_memory" />
            <button id="refresh_cpu">刷新CPU</button>
            <button id="refresh_memory">刷新内存</button>
        </div>
    );
}

export default RealtimeDataCharts;