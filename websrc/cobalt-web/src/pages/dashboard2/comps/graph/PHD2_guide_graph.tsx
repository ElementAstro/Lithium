import * as React from 'react';
import ReactECharts from 'echarts-for-react';
import { GlobalStore } from '../../../../store/globalStore';


const PHD2GuideResultGraph: React.FC = () => {
  const process_data = GlobalStore.useAppState((state) => state.ProcessDataSaveStore);
  const eChartsRef = React.useRef(null as any);
  const [guide_result_graph, set_guide_result_graph] = React.useState<any>(
    {
      tooltip: {
        trigger: 'axis'
      },
      xAxis: {
        type: 'category',
      },
      yAxis: [{
          name: 'error',
          type: 'value'
        },
        {
          name: 'guide',
          type: 'value',
        }
      ],
      series: [{
        data: process_data.PHD2_guide_data_list.RaDistance,
        type: 'line',
        yAxisIndex: 0,
        markLine: {
          silent: true,
          data: []
        },
        lineStyle: {
          color: '#fc0303'
        }
      }, {
        data: process_data.PHD2_guide_data_list.DecDistance,
        type: 'line',
        yAxisIndex: 0,
        markLine: {
          silent: true,
          data: []
        },
        lineStyle: {
          color: '#031cfc'
        }
      }, {
        data: process_data.PHD2_guide_data_list.RaControl,
        type: 'bar',
        yAxisIndex: 1,
        stack: 'Total',
        markLine: {
          silent: true,
          data: []
        },
        itemStyle: {
          borderColor: '#fc0303',
          color: 'rgba(255, 255, 255, 0)'
        }
      }, {
        data: process_data.PHD2_guide_data_list.DecControl,
        type: 'bar',
        yAxisIndex: 1,
        stack: 'Total',
        markLine: {
          silent: true,
          data: []
        },
        itemStyle: {
          borderColor: '#031cfc',
          color: 'rgba(255, 255, 255, 0)'
        }
      }]
    }

  );

  React.useEffect(() => {
    let new_series = guide_result_graph.series;
    new_series[0].data = process_data.PHD2_guide_data_list.RaDistance;
    new_series[1].data = process_data.PHD2_guide_data_list.DecDistance;
    new_series[2].data = process_data.PHD2_guide_data_list.RaControl;
    new_series[3].data = process_data.PHD2_guide_data_list.DecControl;
    set_guide_result_graph({
      ...guide_result_graph,
      series: new_series,
    })
    if (eChartsRef && eChartsRef.current)
      eChartsRef.current?.getEchartsInstance().setOption(guide_result_graph);
  }, [process_data.PHD2_guide_data_list.RaDistance])

  return (
      <ReactECharts
          option={guide_result_graph}
          ref={eChartsRef}
          // style={{height: "200px", margin: 0}}
          // onChartReady={this.onChartReadyCallback}
          // onEvents={EventsDict}
          // opts={}
      />
  )
}
export default PHD2GuideResultGraph;
