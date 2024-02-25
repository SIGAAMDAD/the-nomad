using TheNomad;

namespace TheNomad.Renderer
{
    class RenderEntityReference
    {
        RenderEntityReference() {
        }

        public vec3 pos;
    };

    class RenderSceneReference
    {
        RenderSceneReference() {
        }

        public void ClearScene();
        public void RenderScene();

        public void AddEntityToScene();

        public int m_nWidth;
        public int m_nHeight;
        public uint m_Flags;
        public int m_nX;
        public int m_nY;
    };

};