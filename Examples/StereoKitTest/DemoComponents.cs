using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoComponents : IScene
{
    EntityId floor;
    ComId<ComLifetimeChecker> lifetime;
    Model m;

    public void Initialize()
    {
        string root    = "../Examples/Assets/";
        Tex2D  cubemap = Tex2D.FromEquirectangular(root + "Sky/sky.hdr");
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"  ] = new Tex2D(root + "test.png");
        floorMat["normal"   ] = new Tex2D(root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        m = new Model(Mesh.GenerateCube("app/mesh_cube", Vec3.One), floorMat);
        //int x=0,y=0;
        for (int y = 0; y < 100; y++)
        {
            for (int x = 0; x < 100; x++)
            {
                floor = Entity.Create("Floor");
                floor.Add(new ComRender(m, new Vec3(x,0,y)));
                floor.Add(new ComRender(m, new Vec3(x, 0, y)));
                lifetime = floor.Add(new ComLifetimeChecker());
            }
        }
    }

    public void Shutdown()
    {
    }

    public void Update()
    {
        lifetime.Enabled = Input.Hand(Handed.Right).IsPinched;
        if (Input.Hand(Handed.Right).IsJustPinched)
        {
            floor.Get<ComLifetimeChecker>().With((ref ComLifetimeChecker life) => 
                life.counter += 1
            );
        }
    }
}
