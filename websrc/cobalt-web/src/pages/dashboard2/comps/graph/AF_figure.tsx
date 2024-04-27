import * as React from 'react';
import ReactECharts from 'echarts-for-react';

interface AF_figure_Props {
    focus_measures: Array<[number, number]>
    focus_model: any;
    focus_point: [number, number];
    focus_find: boolean;
}

const AF_figure: React.FC<AF_figure_Props> = (props) => {
    const [focus_figure, set_focus_figure] = React.useState<any>({

    })
    return (
        <></>
    )
}
export default AF_figure;