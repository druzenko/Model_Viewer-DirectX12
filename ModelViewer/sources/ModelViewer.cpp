#include <iostream>
#include <Core.h>

class ModelViewer : public Core::IApp
{
private:

public:

	ModelViewer() {}

	void Startup() override;
	void Cleanup() override;
	void Update(float deltaT) override;
	void RenderScene() override;
};

CREATE_APPLICATION(ModelViewer)

void ModelViewer::Startup()
{

}

void ModelViewer::Cleanup()
{

}

void ModelViewer::Update(float deltaT)
{

}

void ModelViewer::RenderScene()
{

}
