using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoComponents : IScene
{
    Entity floor;

    public void Initialize()
    {
        string root = "../Examples/Assets/";
        Tex2D cubemap = Tex2D.FromEquirectangular(root + "Sky/sky.hdr");
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"] = new Tex2D(root + "test.png");
        floorMat["normal"] = new Tex2D(root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        floor = new Entity("Floor");
        floor.Add(new ComRender(new Model(Mesh.GenerateCube("app/mesh_cube", Vec3.One), floorMat)));
        floor.Add(new ComLifetimeChecker());
    }

    public void Shutdown()
    {
    }

    public void Update()
    {
    }
}
