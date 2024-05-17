import { Action ,action} from 'easy-peasy';

export interface VisibleModel {
    drawerVisible: boolean;
    setState:Action<VisibleModel,Partial<VisibleModel>>;
}

export const getVisibleModel =():VisibleModel =>({
    drawerVisible:false,
    setState:action((state,payload)=>{
        state = Object.assign(state,payload)
    })
})
