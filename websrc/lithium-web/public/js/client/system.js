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

const ctx_cpu = document.getElementById('realtime_cpu').getContext('2d');
const RealtimeCPU = new Chart(ctx_cpu, {
    type: 'line',
    data: {
        datasets: [{
            label: 'CPU用量 %',
            backgroundColor: ['rgba(76, 209, 55, 0.1)'],
            borderColor: 'blue',
            borderWidth: 1
        }],
    }, options: {
        title: {
            display: false
        },
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true,
                    min: 0,
                    max: 100,
                }
            }],
            xAxes: [{
                display: true
            }],
        },
    },
});

const ctx_memory = document.getElementById('realtime_memory').getContext('2d');
const RealtimeMemory = new Chart(ctx_memory, {
    type: 'line',
    data: {
        datasets: [{
            label: '内存用量 %',
            backgroundColor: ['rgba(76, 209, 55, 0.2)'],
            borderColor: 'lightgreen',
            borderWidth: 1
        }],
    }, options: {
        title: {
            display: false
        },
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero: true,
                    min: 0,
                    max: 100,
                }
            }],
            xAxes: [{
                display: true
            }],
        },
    },
});
 
function formatDate(time){
	var date = new Date(time);
 
	var hour = date.getHours(),
		min = date.getMinutes(),
		sec = date.getSeconds();
	var newTime = hour + ':' +
				min + ':' +
				sec;
	return newTime;			
}
 
// update RealtimeCPU
function addCpuData(data) {
    if (RealtimeCPU.data.labels.length>10){
    	RealtimeCPU.data.labels.shift();		
    }
    RealtimeCPU.data.labels.push(formatDate(new Date().getTime()));
    RealtimeCPU.data.datasets.forEach((dataset) => {
        if(data === 0) {
            dataset.data.push(dataset.data[dataset.data.length - 1]);
        } else {
        		if (dataset.data.length>10){
        			dataset.data.shift();         	
            }
            dataset.data.push(data);
        };
    });
    RealtimeCPU.update();
};

// update RealtimeMemory
function addMemoryData(data) {
    if (RealtimeMemory.data.labels.length>10){
    	RealtimeMemory.data.labels.shift();		
    }
    RealtimeMemory.data.labels.push(formatDate(new Date().getTime()));
    RealtimeMemory.data.datasets.forEach((dataset) => {
        if(data === 0) {
            dataset.data.push(dataset.data[dataset.data.length - 1]);
        } else {
        		if (dataset.data.length>10){
        			dataset.data.shift();         	
            }
            dataset.data.push(data);
        };
    });
    RealtimeMemory.update();
};

function getMemoryData(){
    $.ajax({
        url: '/system/api/memory',
        type: 'GET',
        async: true,
        processData: false,
        contentType: "application/json",
        success: function(data){
            addMemoryData(data.used);
        },
        error: function(){
            clearInterval(memory_update);
            $("#errorMessage").text("无法获取内存数据");
            $("#centralModalDanger").modal("show");
        }
    })
};

function getCpuData(){
    $.ajax({
        url: '/system/api/cpu',
        type: 'GET',
        async: true,
        processData: false,
        contentType: "application/json",
        success: function(data){
            addCpuData(data.used);
        },
        error: function(){
            clearInterval(cpu_update);
            $("#errorMessage").text("无法获取CPU数据");
            $("#centralModalDanger").modal("show");
        }
    })
};
 
var memory_update = setInterval(getMemoryData, 5000);
var cpu_update = setInterval(getCpuData, 5000);

document.getElementById("refresh_cpu").addEventListener("click", function(){
    cpu_update = setInterval(getCpuData,5000)
})
document.getElementById("refresh_memory").addEventListener("click", function(){
   memory_update = setInterval(getMemoryData,5000)
})