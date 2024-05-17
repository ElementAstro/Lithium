import * as React from 'react';
import { Alert } from 'react-bootstrap';
import { GlobalStore } from '../../../../store/globalStore';

const InfoUpdateComp: React.FC = () => {
    const state = GlobalStore.useAppState(state=>state.console);

    return (
        <Alert variant={state.alert_type} style={{
            width: "61vw",
            position: "absolute",
            bottom: 0,
            right: 0,
            borderRadius: '8px'
        }}>{state.alert_message}</Alert>
    )
}

export default InfoUpdateComp;
