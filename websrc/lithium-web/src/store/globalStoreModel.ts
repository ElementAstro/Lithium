import {Action, action, createStore,State} from 'easy-peasy';
import { VisibleModel, getVisibleModel } from "./visibleModel";
import { ConnectModel, getConnectModel } from './connectModel';
import { ConsoleModel, getConsoleModel } from './consoleModel';
import { IGLobalParametersModels, GlobalParameterStore } from './GlobalParameterStore';
import { ISTargetListModels, ScheduledTargetListStore } from './ScheduledTargetList';
import {assignDeep} from './persistence'

interface GlobalStoreStateModel {
    visible:VisibleModel
    connect:ConnectModel
    console:ConsoleModel
    GlobalParameterStore: IGLobalParametersModels
    TargetListStore: ISTargetListModels
}

export interface GlobalStoreModel extends GlobalStoreStateModel {
    reset:Action<this, DeepPartial<State<GlobalStoreStateModel>>>
}


export const getGLobalStoreModel = ():GlobalStoreModel=>({
    visible:getVisibleModel(),
    connect:getConnectModel(),
    console:getConsoleModel(),
    GlobalParameterStore: GlobalParameterStore(),
    TargetListStore: ScheduledTargetListStore(),
    reset:action((_,state)=>{
        const result = createStore(getGLobalStoreModel()).getState();
        assignDeep(result,state);
        return result;
    })
});

export type GlobalModelState = State<GlobalStoreModel>;