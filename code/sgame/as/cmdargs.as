namespace TheNomad::Engine {
    shared class CmdArgs {
        CmdArgs() {
        }

        void SendCommand( const string& in cmd ) {
            Cbuf_ExecuteText( cmd );
        }
        void Argc() const {
            return CmdArgc();
        }
        const string& Argv( uint nIndex ) const {
            return CmdArgv( nIndex );
        }
        const string& Args( uint nStartFrom ) const {
            return CmdArgs( nStartFrom );
        }
    };
};