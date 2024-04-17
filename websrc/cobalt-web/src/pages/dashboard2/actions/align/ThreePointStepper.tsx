import * as React from 'react';
import Stepper from '@mui/joy/Stepper';
import Step from '@mui/joy/Step';
import StepIndicator from '@mui/joy/StepIndicator';
import PendingIcon from '@mui/icons-material/Pending';
import CircularProgress from '@mui/joy/CircularProgress';
import CheckRoundedIcon  from '@mui/icons-material/CheckCircle';
import StopIcon from '@mui/icons-material/Stop';
import CancelIcon from '@mui/icons-material/Cancel';
import Typography from '@mui/joy/Typography';
import Stack from '@mui/joy/Stack';
import { useImmer } from 'use-immer';
import { Paper } from '@mui/material';
import { ColorPaletteProp } from '@mui/joy';

import ThreePointDataDisplay from './ThreePointDataDisplay';


interface ThreePointStepperProps {
    current_step: number;
    incoming_data: any;
}
/*
step -1: before first
step 0: initial finished
step 91: manual start move telescope
step 1: start exposure
step 11: exposure and solve finished
step 15: start telescope move
step 16: move finished
step 17: settle finished
step 19: first exposure solve failed
step 2: start second exposure
step 21: exposure and solve finished
step 29: second exposure solve failed
step 3: start thrid exposure
step 31: exposure and solve finished
step 38: error calculation finished
step 39: third exposure solve failed
step 4: update exposure start
step 41: update exposure and solved finished
step 42: update exposure new error calculated
step 49: update exposure solve failed
step 99: update timeout
*/

interface StepProps {
    step_complete_status: number;
    step_exposure_fig_status: number;
    step_number: string;
    step_data: any;
}

const MoveStep:React.FC<StepProps> = (props) => {
    const [completed, set_completed] = React.useState(false);
    const [active, set_active] = React.useState(false);
    const [step_color, set_step_color] = React.useState<ColorPaletteProp>('primary');


    React.useEffect(() => {
        if (props.step_complete_status == 0){
            set_completed(false);
            set_active(true);
            set_step_color('primary');
        }else if (props.step_complete_status == 1){
            set_completed(true);
            set_active(false);
            set_step_color('success');
        }else if (props.step_complete_status == -1){
            set_completed(false);
            set_active(false);
            set_step_color('danger');
        }
    }, [props.step_complete_status])

    return (
        <Step
            completed={completed}
            active={active}
            indicator={
                <StepIndicator variant="solid" color={step_color}>
                    {(props.step_exposure_fig_status == 0) && (<PendingIcon />)}
                    {(props.step_exposure_fig_status == 1) && (<CircularProgress size="sm" variant="solid" />)}
                    {(props.step_exposure_fig_status == 2) && (<CheckRoundedIcon />)}
                    {(props.step_exposure_fig_status == 3) && (<CancelIcon />)}
                </StepIndicator>
            }
        >
            移动望远镜
        </Step>
    )
}

const ExposureStep:React.FC<StepProps> = (props) => {
    const [completed, set_completed] = React.useState(false);
    const [active, set_active] = React.useState(false);
    const [step_color, set_step_color] = React.useState<ColorPaletteProp>('primary');

    React.useEffect(() => {
        if (props.step_complete_status == 0){
            set_completed(false);
            set_active(true);
            set_step_color('primary');
        }else if (props.step_complete_status == 1){
            set_completed(true);
            set_active(false);
            set_step_color('success');
        }else if (props.step_complete_status == -1){
            set_completed(false);
            set_active(false);
            set_step_color('danger');
        }
    }, [props.step_complete_status])


    return (
        <Step
            completed={completed}
            active={active}
            indicator={
                <StepIndicator variant="solid" color={step_color}>
                    {(props.step_exposure_fig_status == 0) && (<PendingIcon />)}
                    {(props.step_exposure_fig_status == 1) && (<CircularProgress size="sm" variant="solid" />)}
                    {(props.step_exposure_fig_status == 2) && (<CheckRoundedIcon />)}
                    {(props.step_exposure_fig_status == 3) && (<CancelIcon />)}
                </StepIndicator>
            }
        >
            拍摄解析第{props.step_number}张
        </Step>
    )
}


const ThreePointStepper:React.FC<ThreePointStepperProps> = (props)=>{
    // ui control
    const [measure_finished, set_measure_finished] = React.useState(false);
    const [step_show_data, update_step_show_data] = useImmer({
        first_exposure_complete_status: 0, // 0 for normal, 1 for complete, -1 for fail
        first_exposure_fig_status: 0, // 0 for waiting, 1 for in progress, 2 for finished, 4 for failed
        first_move_complete_status: 0,
        first_move_fig_status: 0,
        second_exposure_complete_status: 0,
        second_exposure_fig_status: 0,
        second_move_complete_status: 0,
        second_move_fig_status: 0,
        third_exposure_complete_status: 0,
        third_exposure_fig_status: 0,
        update_exposure_complete_status: 0,
        update_exposure_fig_status: 0,
    })
    const [current_discription, set_current_discription] = React.useState('');
    const [first_move, set_first_move] = React.useState(false);

    // polar error related data
    const [east_west_angle, set_east_west_angle] = React.useState<number | null>(null);
    const [up_down_angle, set_up_down_angle] = React.useState<number | null>(null);
    const [target_ra_dec, set_target_ra_dec] = React.useState<ICThreePointTarget | null>(null);
    const [current_ra_dec, set_current_ra_dec] = React.useState<ICThreePointTarget | null>(null);
    const [solved_ra_dec, update_solved_ra_dec] = useImmer<[ICThreePointTarget | null, ICThreePointTarget | null, ICThreePointTarget | null, ICThreePointTarget | null]>([null, null, null, null]);

    // use effect
    // on mount
    React.useEffect(() => {

    }, []);
    React.useEffect(() => {
        switch (props.current_step){
            case (-1): {
                set_measure_finished(false);
                update_step_show_data((draft) => {
                    draft.first_exposure_complete_status = 0;  // 0 for normal, 1 for complete, -1 for fail
                    draft.first_exposure_fig_status = 0; // 0 for waiting, 1 for in progress, 2 for finished, 4 for failed
                    draft.first_move_complete_status = 0;
                    draft.first_move_fig_status = 0;
                    draft.second_exposure_complete_status = 0;
                    draft.second_exposure_fig_status = 0;
                    draft.second_move_complete_status = 0;
                    draft.second_move_fig_status = 0;
                    draft.third_exposure_complete_status = 0;
                    draft.third_exposure_fig_status = 0;
                    draft.update_exposure_complete_status = 0;
                    draft.update_exposure_fig_status = 0;
                });
                update_solved_ra_dec((draft) => {
                    draft = [null, null, null, null];
                });
                set_current_discription('等待启动');
                break;
            }
            case (91): {
                set_measure_finished(false);
                set_first_move(true);
                break;
            }
            case (0): {
                set_measure_finished(false);
                update_step_show_data((draft) => {
                    draft.first_exposure_complete_status = 0;  // 0 for normal, 1 for complete, -1 for fail
                    draft.first_exposure_fig_status = 0; // 0 for waiting, 1 for in progress, 2 for finished, 4 for failed
                    draft.first_move_complete_status = 0;
                    draft.first_move_fig_status = 0;
                    draft.second_exposure_complete_status = 0;
                    draft.second_exposure_fig_status = 0;
                    draft.second_move_complete_status = 0;
                    draft.second_move_fig_status = 0;
                    draft.third_exposure_complete_status = 0;
                    draft.third_exposure_fig_status = 0;
                    draft.update_exposure_complete_status = 0;
                    draft.update_exposure_fig_status = 0;
                });
                update_solved_ra_dec((draft) => {
                    draft = [null, null, null, null];
                });
                set_first_move(true);
                set_current_discription('初始化设置完成');
                break;
            }
            case (1): { // step 1: start exposure
                set_first_move(true);
                update_step_show_data((draft) => {
                    draft.first_exposure_complete_status = 0;
                    draft.first_exposure_fig_status = 1;
                });
                set_current_discription('开始第一次曝光');
                break;
            }
            case (11): { // step 11: exposure and solve finished
                update_step_show_data((draft) => {
                    draft.first_exposure_complete_status = 1;
                    draft.first_exposure_fig_status = 2;
                });
                set_current_discription('第一张曝光解析完成');
                update_solved_ra_dec((draft) => {
                    draft[0] = {
                        ra: props.incoming_data.ra,
                        dec: props.incoming_data.dec,
                    }
                })
                break;
            }
            case (15): { // step 15: starttelescope move
                if (first_move){
                    update_step_show_data((draft) => {
                        draft.first_move_complete_status = 0;
                        draft.first_move_fig_status = 1;
                    });
                }else{
                    update_step_show_data((draft) => {
                        draft.second_move_complete_status = 0;
                        draft.second_move_fig_status = 1;
                    });
                }
                set_current_discription('移动望远镜中');
                break;
            }
            case (16): { // step 16: move finished
                if (first_move){
                    update_step_show_data((draft) => {
                        draft.first_move_complete_status = 1;
                        draft.first_move_fig_status = 1;
                    });
                }else{
                    update_step_show_data((draft) => {
                        draft.second_move_complete_status = 1;
                        draft.second_move_fig_status = 1;
                    });
                }
                set_current_discription('等待望远镜稳定中');
                break;
            }
            case (17): { // step 17: settle finished
                if (first_move){
                    update_step_show_data((draft) => {
                        draft.first_move_complete_status = 1;
                        draft.first_move_fig_status = 2;
                    });
                }else{
                    update_step_show_data((draft) => {
                        draft.second_move_complete_status = 1;
                        draft.second_move_fig_status = 2;
                    });
                }
                set_current_discription('望远镜稳定完成');
                break;
            }
            case (19): { // step 19: first exposure solve failed
                update_step_show_data((draft) => {
                    draft.first_exposure_complete_status = -1;
                    draft.first_exposure_fig_status = 4;
                });
                set_current_discription('第一次曝光解析失败，终止流程');
                break;
            }
            case (2): { // step 2: start second exposure
                set_first_move(false);
                update_step_show_data((draft) => {
                    draft.second_exposure_complete_status = 0;
                    draft.second_exposure_fig_status = 1;
                });
                set_current_discription('开始第二次曝光');
                break;
            }
            case (21): { // step 21: exposure and solve finished
                update_step_show_data((draft) => {
                    draft.second_exposure_complete_status = 1;
                    draft.second_exposure_fig_status = 2;
                });
                set_current_discription('第二张曝光解析完成');
                update_solved_ra_dec((draft) => {
                    draft[1] = {
                        ra: props.incoming_data.ra,
                        dec: props.incoming_data.dec,
                    }
                })
                break;
            }
            case (25): { // step 25: starttelescope move
                update_step_show_data((draft) => {
                    draft.second_move_complete_status = 0;
                    draft.second_move_fig_status = 1;
                });
                set_current_discription('移动望远镜中');
                break;
            }
            case (26): { // step 26: move finished
                set_current_discription('等待望远镜稳定中');
                break;
            }
            case (27): { // step 27: settle finished
                update_step_show_data((draft) => {
                    draft.second_move_complete_status = 1;
                    draft.second_move_fig_status = 2;
                });
                set_current_discription('望远镜稳定完成');
                break;
            }
            case (29): { // step 29: second exposure solve failed
                update_step_show_data((draft) => {
                    draft.second_exposure_complete_status = -1;
                    draft.second_exposure_fig_status = 4;
                });
                set_current_discription('第二次曝光解析失败，终止流程');
                break;
            }
            case (3): { // step 3: start thrid exposure
                update_step_show_data((draft) => {
                    draft.third_exposure_complete_status = 0;
                    draft.third_exposure_fig_status = 1;
                });
                set_current_discription('开始第三次曝光');
                break;
            }
            case (31): { // step 31: exposure and solve finished
                update_step_show_data((draft) => {
                    draft.third_exposure_complete_status = 1;
                    draft.third_exposure_fig_status = 2;
                });
                set_current_discription('第三张曝光解析完成');
                update_solved_ra_dec((draft) => {
                    draft[2] = {
                        ra: props.incoming_data.ra,
                        dec: props.incoming_data.dec,
                    }
                });
                set_current_ra_dec({
                    ra: props.incoming_data.ra,
                    dec: props.incoming_data.dec,
                });
                break;
            }
            case (38): { // step 38: error calculation finished
                set_measure_finished(true);
                set_east_west_angle(props.incoming_data.east_west_move);
                set_up_down_angle(props.incoming_data.up_down_move);
                set_target_ra_dec({
                    ra: props.incoming_data.target_ra,
                    dec: props.incoming_data.target_dec
                });
                break;
            }
            case (39): { // step 39: third exposure solve failed
                update_step_show_data((draft) => {
                    draft.third_exposure_complete_status = -1;
                    draft.third_exposure_fig_status = 4;
                });
                set_current_discription('第三次曝光解析失败，终止流程');
                break;
            }
            case (4): { // step 4: update exposure start
                set_measure_finished(true);
                update_step_show_data((draft) => {
                    draft.update_exposure_complete_status = 0;
                    draft.update_exposure_fig_status = 1;
                });
                set_current_discription('开始修正极轴曝光');
                break;
            }
            case (41): { // step 41: update exposure and solved finished
                set_measure_finished(true);
                update_step_show_data((draft) => {
                    draft.update_exposure_complete_status = 1;
                    draft.update_exposure_fig_status = 2;
                });
                set_current_discription('修正极轴曝光解析完成');
                update_solved_ra_dec((draft) => {
                    draft[3] = {
                        ra: props.incoming_data.ra,
                        dec: props.incoming_data.dec,
                    }
                });
                set_target_ra_dec({
                    ra: props.incoming_data.target_ra,
                    dec: props.incoming_data.target_dec
                });
                break;
            }
            case (42): { // step 42: update exposure new error calculated
                set_measure_finished(true);
                set_east_west_angle(props.incoming_data.east_west_move);
                set_up_down_angle(props.incoming_data.up_down_move);
                break;
            }
            case (49): { // step 49: update exposure solve failed
                update_step_show_data((draft) => {
                    draft.update_exposure_complete_status = -1;
                    draft.update_exposure_fig_status = 4;
                });
                set_current_discription('修正极轴曝光解析失败等待中');
                break;
            }
            case (99): { // update timeout
                break;
            }
            default: {
                break;
            }
        }
    }, [props.current_step])


    return (
        <Paper>
            {
                (measure_finished)?
                (
                    <>
                        <ThreePointDataDisplay east_west_angle={east_west_angle} up_down_angle={up_down_angle} target_radec={target_ra_dec} current_radec={current_ra_dec} />
                        <Stepper sx={{ width: '100%' }} color="white" orientation="vertical">
                            <ExposureStep step_complete_status={step_show_data.update_exposure_complete_status} 
                            step_exposure_fig_status={step_show_data.update_exposure_fig_status} step_number={'N'} step_data={{
                                solved_ra_dec: solved_ra_dec[3]
                            }}/>
                            <Step
                                indicator={
                                    <StepIndicator variant="solid" color="primary">
                                        <StopIcon />
                                    </StepIndicator>
                            }
                            >
                                <Typography>停止</Typography>
                                <Stack spacing={1}>
                                    <Typography level="body-sm">
                                        单次对极轴的时间不建议超过5分钟。 <br />
                                        粗对之后再重启一次流程会更准。 <br />
                                    </Typography>
                                </Stack>
                            </Step>
                        </Stepper>
                    </>
                ):
                (
                    <Stepper sx={{ width: '100%' }} color="white" orientation="vertical">
                        <ExposureStep step_complete_status={step_show_data.first_exposure_complete_status} 
                        step_exposure_fig_status={step_show_data.first_exposure_fig_status} step_number={'一'} step_data={{
                            solved_ra_dec: solved_ra_dec[0]
                        }}/>

                        <MoveStep step_complete_status={step_show_data.first_move_complete_status} 
                        step_exposure_fig_status={step_show_data.first_move_fig_status} step_number={'一'} step_data={{}} />

                        <ExposureStep step_complete_status={step_show_data.second_exposure_complete_status} 
                        step_exposure_fig_status={step_show_data.second_exposure_fig_status} step_number={'二'} step_data={{
                            solved_ra_dec: solved_ra_dec[1]
                        }}/>

                        <MoveStep step_complete_status={step_show_data.second_move_complete_status} 
                        step_exposure_fig_status={step_show_data.second_move_fig_status} step_number={'二'} step_data={{}} />

                        <ExposureStep step_complete_status={step_show_data.third_exposure_complete_status} 
                        step_exposure_fig_status={step_show_data.third_exposure_fig_status} step_number={'三'} step_data={{
                            solved_ra_dec: solved_ra_dec[2]
                        }}/>
                        
                        <Step
                            indicator={
                                <StepIndicator variant="solid" color="primary">
                                    <PendingIcon />
                                </StepIndicator>
                            }
                        >计算误差</Step>

                    </Stepper>
                )
            }
            
        </Paper>
    )
}

export default ThreePointStepper;