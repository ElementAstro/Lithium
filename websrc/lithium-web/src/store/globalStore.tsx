import * as React from 'react';
import {createStore,createTypedHooks,StoreProvider} from 'easy-peasy';
import {Middleware} from 'redux';
import {getGLobalStoreModel,GlobalStoreModel} from './globalStoreModel';

function createGlobalStore(){
    const middleware:Middleware[] = [];
    const store = createStore(getGLobalStoreModel(),{middleware});
    return store;
}

const hooks = createTypedHooks<GlobalStoreModel>();


let _globalStoreInstance:ReturnType<typeof createGlobalStore> |undefined;

const globalStoreInstance = ():ReturnType<typeof createGlobalStore>=>{
    if(_globalStoreInstance===undefined){
        _globalStoreInstance = createGlobalStore();
    }
    return _globalStoreInstance;
}

export const GlobalStore = {
    useAppState:hooks.useStoreState,
    get actions(){
        return globalStoreInstance().getActions();
    }
}

export const GlobalStoreProvider:React.FC<any>=({children})=>{
    return(
        <StoreProvider store={globalStoreInstance()}>{children}</StoreProvider>
    )
}


