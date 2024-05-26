import { GlobalStoreProvider } from './store/globalStore';
import { combineProviders } from './utils/combineProviders';
import { EchoWebSocketProvider } from './utils/websocketProvider';
import { CustomThemeProvider } from './theme';

export const AppProviders = ({ children }: { children?: React.ReactNode }) => combineProviders([
    GlobalStoreProvider,
    EchoWebSocketProvider,
], children);
