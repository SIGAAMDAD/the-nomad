namespace TheNomad::Util {
    shared class JsonObject {
        JsonObject() {
        }
        JsonObject( const JsonObject& in other ) {
            this = other;
        }
        JsonObject( const string& in fileName ) {
            handle = JsonParser( fileName );
        }

        bool Parse( const string& in fileName ) {
            return handle.Parse( fileName );
        }

        void SetInt( const string& in name, int v ) {
            handle.SetInt( name, v );
        }
        void SetUInt( const string& in name, uint v ) {
            handle.SetUInt( name, v );
        }
        void SetFloat( const string& in name, float v ) {
            handle.SetFloat( name, v );
        }
        void SetString( const string& in name, const string& in v ) {
            handle.SetString( name, v );
        }

        bool GetVec2( const string& in name, vec2& out v ) {
            float[] vec( 2 );
            return handle.GetFloatArray( name, vec );
        }
        bool GetVec3( const string& in name, vec3& out v ) {
            float[] vec( 3 );
            return handle.GetFloatArray( name, vec );
        }
        bool GetVec4( const string& in name, vec4& out v ) {
            float[] vec( 4 );
            return handle.GetFloatArray( name, vec );
        }
        bool GetInt( const string& in name, int& out v ) {
            return handle.GetInt( name, v );
        }
        bool GetUInt( const string& in name, uint& out v ) {
            return handle.GetUInt( name, v );
        }
        bool GetFloat( const string& in name, float& out v ) {
            return handle.GetFloat( name, v );
        }
        bool GetString( const string& in name, string& out v ) {
            return handle.GetString( name, v );
        }
        bool GetBool( const string& in name, bool& out v ) {
            return handle.GetBool( name, v );
        }
        bool GetObject( const string& in name, JsonObject& out v ) {
            return handle.GetObject( name, v.handle );
        }
        bool GetIntArray( const string& in name, array<int>& out v ) {
            return handle.GetIntArray( name, v );
        }
        bool GetUIntArray( const string& in name, array<uint>& out v ) {
            return handle.GetUIntArray( name, v );
        }
        bool GetFloatArray( const string& in name, array<float>& out v ) {
            return handle.GetFloatArray( name, v );
        }
        bool GetStringArray( const string& in name, array<string>& out v ) {
            return handle.GetStringArray( name, v );
        }
        bool GetObjectArray( const string& in name, array<JsonObject>& out v ) {
            return handle.GetObjectArray( name, v );
        }

        private JsonParser handle;
    };
};