import {useCallback} from 'react';
import {List} from 'react-bootstrap-icons';
import {GlobalStore} from '../../store/globalStore';

import './style.less';
import React from 'react';

const Header = ()=>{

    const {drawerVisible} = GlobalStore.useAppState(state=>state.visible);

    const toggleNavigation = useCallback(()=>{
        GlobalStore.actions.visible.setState({
            drawerVisible:!drawerVisible
        })
    },[drawerVisible]);

    return (
        <div className='app-header'>
            <List onClick={toggleNavigation}/>
            <span className='app-header-title'>Cobalt</span>
        </div>
    )
}

export default Header;