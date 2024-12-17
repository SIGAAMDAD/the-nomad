namespace TheNomad::SGame {
	class WallObject : EntityObject {
		WallObject() {
		}

		void Spawn( uint id, const vec3& in origin ) override {
			m_Link.m_Origin = origin;

			@m_State = @StateManager.GetNullState();
		}
		void Think() override {
			@m_State = @StateManager.GetNullState();
		}
	};
};