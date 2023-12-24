import { createStore, action, thunk, computed } from 'easy-peasy';
import {Thunk, Action, Computed } from 'easy-peasy';
import { result } from 'lodash';

interface update_string {
    index: number,
    update_string: string,
}

const all_available_tags = [
    '星云', '星系', '黑白目标', '彩机目标', 'LRGB', 'HSO', '双窄'
]

export interface ISTargetListModels {
    current_focus_target: IDSOObjectInfo;
    all_saved_targets: Array<IDSOFramingObjectInfo>;
    add_target: Action<ISTargetListModels, IDSOFramingObjectInfo | IDSOObjectInfo>;
    remove_target: Action<ISTargetListModels, number>;
    target_set_flag: Action<ISTargetListModels, update_string>;
    target_set_tag: Action<ISTargetListModels, update_string>;
    update_focus_target: Action<ISTargetListModels, IDSOObjectInfo>;
    clear_focus_target: Action<ISTargetListModels>;
    // get_targets_by_flag: Action<ISTargetListModels, string>;
    // get_targets_by_tag: Action<ISTargetListModels, string>;
    // load_user_local_data
}

// todo, load user local saved data.
export const ScheduledTargetListStore = ():ISTargetListModels =>({
    current_focus_target: {name: '', ra: 0, dec: 0},
    all_saved_targets: [],
    add_target: action((state, payload) => {
    }),
    remove_target: action((state, payload) => {
        if (payload < state.all_saved_targets.length){
            state.all_saved_targets.splice(payload, 1);  // todo check js list remove is available?
        }
    }),
    target_set_flag: action((state, payload) => {
        if (payload.index < state.all_saved_targets.length){
            state.all_saved_targets[payload.index].flag = payload.update_string;
        }
    }),
    target_set_tag: action((state, payload) => {
        if (payload.index < state.all_saved_targets.length){
            state.all_saved_targets[payload.index].tag = payload.update_string;
        }
    }),
    update_focus_target: action((state, payload) => {
        state.current_focus_target = payload;
    }),
    clear_focus_target: action((state)=>{
        state.current_focus_target = {
            name: '-', ra: 0, dec: 0,
        }
    })
    // get_targets_by_flag: action((state, payload) => {
    //     let result = [];
    //     for(let i=0;i<state.all_saved_targets.length;i++){
    //         if (payload == state.all_saved_targets[i].flag){
    //             result.push(state.all_saved_targets[i]);
    //         }
    //     }
    //     return result;
    // }),
    // get_targets_by_tag: action((state, payload) => {
        
    // }),
})