import { Action ,action} from 'easy-peasy';

export interface ConsoleModel {
    menu: 'star'|'shoot'|'align';
    drawerVisible:boolean;
    setState:Action<ConsoleModel,Partial<ConsoleModel>>;
}

export const getConsoleModel =():ConsoleModel =>({
    menu:'star',
    drawerVisible:true,
    setState:action((state,payload)=>{
        state = Object.assign(state,payload)
    })
})






