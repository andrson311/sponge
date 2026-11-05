#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imGuIZMOquat.h"

#include "core_scene.h"
#include "core_rendering_system.h"

#define NUM_SCENE_OBJECTS 1024

SceneObject::SceneObject()
{
    memset(&m_rotations[0], 0, sizeof(m_rotations));
}

void SceneObject::SetRotation(const glm::vec3 &rot)
{
    m_rotations[0] = rot;
    m_numRotations = 1;
}

void SceneObject::SetRotation(float x, float y, float z)
{
    m_rotations[0].x = x;
    m_rotations[0].y = y;
    m_rotations[0].z = z;
    m_numRotations = 1;
}

void SceneObject::RotateBy(float x, float y, float z)
{
    m_rotations[0].x += x;
    m_rotations[0].y += y;
    m_rotations[0].z += z;
    m_numRotations = 1;
}

void SceneObject::PushRotation(const glm::vec3 &rot)
{
    if (m_numRotations >= MAX_NUM_ROTATIONS)
    {
        printf("Exceeded max number of rotations - %d\n", m_numRotations);
        assert(0);
    }

    m_rotations[m_numRotations] = rot;
    m_numRotations++;
}

bool QuaternionIsZero(const glm::quat &q)
{
    return (q.x == 0 && q.y == 0 && q.z == 0.0f && q.w == 0.0f);
}

glm::mat4 SceneObject::GetMatrix() const
{
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

    glm::mat4 rotation;

    if (QuaternionIsZero(m_quaternion))
    {
        CalcRotationStack(rotation);
    }
    else
    {
        rotation = glm::mat4_cast(m_quaternion);
    }

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_pos);

    return translation * rotation * scale;
}

void SceneObject::CalcRotationStack(glm::mat4 &rot) const
{
    if (m_numRotations == 0)
    {
        rot = glm::mat4(1.0f);
    }
    else
    {
        rot = glm::eulerAngleZYX(glm::radians(m_rotations[0].x),
                                 glm::radians(m_rotations[0].y),
                                 glm::radians(m_rotations[0].z));

        if (m_numRotations > MAX_NUM_ROTATIONS)
        {
            printf("Invalid number of rotations - %d\n", m_numRotations);
            assert(0);
        }

        for (int i = 1; i < m_numRotations; i++)
        {
            glm::mat4 r = glm::eulerAngleZYX(glm::radians(m_rotations[i].x),
                                             glm::radians(m_rotations[i].y),
                                             glm::radians(m_rotations[i].z));
            rot = r * rot;
        }
    }
}

glm::vec3 SceneObject::GetForwardDir() const
{
    glm::vec3 ret;

    if (QuaternionIsZero(m_quaternion))
    {
        if (m_numRotations == 0)
        {
            ret = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        else
        {
            if (m_numRotations > 1)
            {
                printf("Getting the forward direction with multiple rotations is unimplemented\n");
                assert(0);
                exit(1);
            }

            ret.x = -sinf(glm::radians(m_rotations[0].y));
            ret.y = 0.0f;
            ret.z = cosf(glm::radians(m_rotations[0].y));
        }
    }
    else
    {
        glm::vec3 defaultForward(0.0f, 0.0f, 1.0f);
        glm::vec3 forward = m_quaternion * defaultForward;
        ret = forward;
    }

    if (ret != glm::vec3(0.0f))
    {
        ret = glm::normalize(ret);
    }

    return ret;
}

SceneConfig::SceneConfig()
{
}

Scene::Scene()
{
}

CoreScene::CoreScene(CoreRenderingSystem *pRenderingSystem)
{
    m_pCoreRenderingSystem = pRenderingSystem;
    CreateDefaultCamera();
    m_sceneObjects.resize(NUM_SCENE_OBJECTS);
}

void CoreScene::LoadScene(const std::string &filename, const ModelLoadFlags &flags)
{
    CoreModel *pModel = (CoreModel *)m_pCoreRenderingSystem->LoadModel(filename.c_str(), flags);
    SceneObject *pSceneObject = CreateSceneObject(pModel);
    AddToRenderList(pSceneObject);

    if (pModel->GetCameras().size() == 0)
    {
        printf("Warning! '%s' does not include a camera, Falling back to default.\n", filename.c_str());
    }
    else
    {
        m_defaultCamera = pModel->GetCameras()[0];
    }

    m_pointLights = pModel->GetPointLights();
    m_spotLights = pModel->GetSpotLights();
    m_dirLights = pModel->GetDirLights();
}

void CoreScene::InitializeDefault()
{
    SceneObject *pSceneObject = CreateSceneObject("square");
    AddToRenderList(pSceneObject);

    pSceneObject->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
    pSceneObject->SetScale(glm::vec3(1000.0f));
    pSceneObject->SetFlatColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
}

void CoreScene::CreateDefaultCamera()
{
    glm::vec3 pos(0.0f);
    glm::vec3 target(0.0f, 0.0f, 1.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.2f;
    float zFar = 20.0f;
    int windowWidth = 0;
    int windowHeight = 0;
    m_pCoreRenderingSystem->GetWindowSize(windowWidth, windowHeight);

    PersProjInfo persProjInfo = {
        FOV,
        (float)windowWidth,
        (float)windowHeight,
        zNear,
        zFar};

    glm::vec3 center = pos + target;
    m_defaultCamera.Init(pos, center, up, persProjInfo);
}

void CoreScene::SetCamera(const glm::vec3 &pos, const glm::vec3 &target)
{
    m_defaultCamera.Init(pos, target);
}

void CoreScene::SetCameraZRange(float near, float far)
{
    m_defaultCamera.Init(near, far);
}

void CoreScene::SetCameraAcceleration(float acceleration)
{
    m_defaultCamera.SetAcceleration(acceleration);
}

/*
void CoreScene::SetCameraSpeed(float speed)
{
}
*/

void CoreScene::AddToRenderList(SceneObject *pSceneObject)
{
    CoreSceneObject *pCoreSceneObject = (CoreSceneObject *)pSceneObject;
    std::list<CoreSceneObject *>::const_iterator it = std::find(
        m_renderList.begin(), m_renderList.end(), pCoreSceneObject);

    if (it == m_renderList.end())
    {
        m_renderList.push_back(pCoreSceneObject);
    }
}

bool CoreScene::RemoveFromRenderList(SceneObject *pSceneObject)
{
    std::list<CoreSceneObject *>::const_iterator it = std::find(
        m_renderList.begin(), m_renderList.end(), pSceneObject);

    if (it != m_renderList.end())
    {
        m_renderList.erase(it);
        return true;
    }

    return false;
}

std::list<SceneObject *> CoreScene::GetSceneObjectsList()
{
    // TODO: not very efficient

    std::list<SceneObject *> ObjectList;

    for (std::list<CoreSceneObject *>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++)
    {
        ObjectList.push_back(*it);
    }

    return ObjectList;
}

void CoreScene::SceneObjectGUI()
{
    if (ImGui::TreeNode("Scene Object"))
    {
        for (std::list<CoreSceneObject *>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++)
        {
            if (ImGui::TreeNode((*it)->GetName().c_str()))
            {
                glm::vec3 pos = (*it)->GetPosition();
                ImGui::Text("Position %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
                ImGui::SliderFloat3("Position", &pos.x, -5.0f, 5.0f);
                (*it)->SetPosition(glm::vec3(pos.x, pos.y, pos.z));
                float scale = (*it)->GetScale();
                ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f);
                (*it)->SetScale(glm::vec3(scale));
                glm::vec3 rot = (*it)->GetRotation();
                ImGui::Text("Rotation %.3f,%.3f,%.3f", rot.x, rot.y, rot.z);
                ImGui::SliderFloat3("Rotation", &rot.x, -180.0f, 180.f);
                (*it)->SetRotation(rot);
                glm::vec3 colorMod = (*it)->GetColorMod();
                ImGui::SliderFloat3("Color Mod", &colorMod.x, 0.0f, 10.0f);
                (*it)->SetColorMod(colorMod);
                int cubeMipmapLevel = (*it)->GetCubeMipmapLevel();
                int maxMipmapLevels = GetSkyboxMipmapLevels();
                ImGui::SliderInt("Cube mipmap level", &cubeMipmapLevel, 0, maxMipmapLevels, "", 0);
                (*it)->SetCubeMipmapLevel(cubeMipmapLevel);
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}

void CoreScene::SSAOGUI()
{
    if (ImGui::TreeNode("SSAO"))
    {
        SSAOParams &ssaoParams = m_config.GetSSAOParams();

        bool enableSSAO = m_config.IsSSAOEnabled();
        ImGui::Checkbox("SSAO Enable", &enableSSAO);
        m_config.ControlSSAO(enableSSAO);
        ImGui::SliderFloat("SSAO Scale", &ssaoParams.scale, 0.0f, 2.0f);
        ImGui::SliderFloat("SSAO Bias", &ssaoParams.bias, 0.0f, 0.3f);
        ImGui::SliderFloat("SSAO Radius", &ssaoParams.radius, 0.05f, 0.5f);
        ImGui::SliderFloat("SSAO OccScale", &ssaoParams.occScale, 0.1f, 50.0f);
        ImGui::SliderFloat("SSAO AttScale", &ssaoParams.attScale, 0.5f, 1.5f);
        ImGui::SliderFloat("SSAO DistScale", &ssaoParams.distScale, 0.0f, 1.0f);
        m_pCoreRenderingSystem->ImGuiTextureWindow("SSAO");

        ImGui::TreePop();
    }
}

void CoreScene::StartSceneGUI(const char *pTitle)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool my_tool_active = false;
    ImGui::Begin(pTitle, &my_tool_active);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    GeneralGUI();
    LightingGUI();
    SceneObjectGUI();
    SSAOGUI();
    HDRAndToneMappingGUI();
    TerrainGUI();
}

void CoreScene::EndSceneGUI()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CoreScene::LightingGUI()
{
    if (ImGui::TreeNode("Lighting"))
    {
        if (m_pointLights.size() > 0)
        {
            if (ImGui::TreeNode("Point Lights"))
            {
                for (int i = 0; i < m_pointLights.size(); i++)
                {
                    PointLight &l = m_pointLights[i];
                    ImGui::ColorEdit3("Color", &l.Color.x);
                    ImGui::SliderFloat("Ambient Intensity", &l.AmbientIntensity, 0.0f, 10.0f);
                    ImGui::SliderFloat("Diffuse Intensity", &l.DiffuseIntensity, 0.0f, 10.0f);
                    glm::vec3 &pos = l.WorldPosition;
                    ImGui::Text("Position: %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
                    ImGui::SliderFloat3("Position", &pos.x, -50.0f, 50.0f);
                }

                ImGui::TreePop();
            }
        }

        if (m_dirLights.size() > 0)
        {
            if (ImGui::TreeNode("Directional Lights"))
            {
                for (int i = 0; i < m_dirLights.size(); i++)
                {
                    std::string str = "[" + std::to_string(i) + "]";
                    if (ImGui::TreeNode(str.c_str()))
                    {
                        DirectionalLight &l = m_dirLights[i];
                        ImGui::ColorEdit3("Color", &l.Color.x);
                        ImGui::SliderFloat("Ambient Intensity", &l.AmbientIntensity, 0.0f, 10.0f);
                        ImGui::SliderFloat("Diffuse Intensity", &l.DiffuseIntensity, 0.0f, 10.0f);
                        glm::vec3 &dir = l.WorldDirection;
                        ImGui::Text("Direction: %.3f, %.3f, %.3f", dir.x, dir.y, dir.z);
                        ImGui::SliderFloat3("Direction", &dir.x, -1.0f, 1.0f);
                        dir = glm::normalize(dir);
                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }
        }

        if (m_spotLights.size() > 0)
        {
            if (ImGui::TreeNode("Spot Lights"))
            {
                for (int i = 0; i < m_spotLights.size(); i++)
                {
                    std::string str = "[" + std::to_string(i) + "]";
                    if (ImGui::TreeNode(str.c_str()))
                    {
                        SpotLight &l = m_spotLights[i];
                        ImGui::ColorEdit3("Color", &l.Color.x);
                        ImGui::SliderFloat("Ambient Intensity", &l.AmbientIntensity, 0.0f, 10.0f);
                        ImGui::SliderFloat("Diffuse Intensity", &l.DiffuseIntensity, 0.0f, 10.0f);
                        glm::vec3 &pos = l.WorldPosition;
                        ImGui::Text("Position: %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
                        ImGui::SliderFloat3("Position", &pos.x, -50.0f, 50.f);
                        glm::vec3 &dir = l.WorldDirection;
                        ImGui::Text("Direction: %.3f, %.3f, %.3f", dir.x, dir.y, dir.z);
                        ImGui::SliderFloat3("Direction", &dir.x, -1.0f, 1.0f);
                        dir = glm::normalize(dir);
                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}

void CoreScene::GeneralGUI()
{
    if (ImGui::TreeNode("General"))
    {
        glm::vec3 cameraPos = m_defaultCamera.GetPosition();
        ImGui::Text("Camera Position: %.3f, %.3f, %.3f", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::SliderFloat3("Camera Position", &cameraPos.x, -1000.0f, 1000.0f);
        m_defaultCamera.SetPosition(cameraPos);

        bool disablePBR = m_config.IsPBRDisabled();
        ImGui::Checkbox("Disable PBR", &disablePBR);
        m_config.ForcePBRDisabled(disablePBR);

        bool enableShadowMapping = m_config.IsShadowMappingEnabled();
        ImGui::Checkbox("Enable Shadow Mapping", &enableShadowMapping);
        m_config.ControlShadowMapping(enableShadowMapping);

        ImGui::Text("Render Mode");
        RENDER_MODE renderMode = m_config.GetRenderMode();
        ImGui::RadioButton("Full", (int *)&renderMode, RENDER_MODE_FULL);
        ImGui::SameLine();
        ImGui::RadioButton("Texture only", (int *)&renderMode, RENDER_MODE_TEXTURE_ONLY);
        ImGui::SameLine();
        ImGui::RadioButton("Lighting only", (int *)&renderMode, RENDER_MODE_LIGHTING_ONLY);
        ImGui::SameLine();
        ImGui::RadioButton("Normals", (int *)&renderMode, RENDER_MODE_NORMALS);
        ImGui::SameLine();
        ImGui::RadioButton("Tex Coords", (int *)&renderMode, RENDER_MODE_TEXCOORDS);
        ImGui::SameLine();
        ImGui::RadioButton("Base Color", (int *)&renderMode, RENDER_MODE_BASE_COLOR);
        ImGui::RadioButton("PBR Diffuse Contribution", (int *)&renderMode, RENDER_MODE_PBR_DIFFUSE_CONTRIB);
        ImGui::SameLine();
        ImGui::RadioButton("PBR F", (int *)&renderMode, RENDER_MODE_PBR_F);
        m_config.SetRenderMode(renderMode);

        ImGui::TreePop();
    }
}

void CoreScene::HDRAndToneMappingGUI()
{
    if (ImGui::TreeNode("HDR & Tone Mapping"))
    {
        bool enableHDR = m_config.IsHDREnabled();
        ImGui::Checkbox("HDR Enabled", &enableHDR);
        m_config.ControlHDR(enableHDR);

        float averageLuminance = 0.0f;
        float exposure = 0.0f;
        m_config.GetHDRParams(averageLuminance, exposure);
        ImGui::Text("HDR average luminance %.4f", averageLuminance);
        ImGui::Text("HDR exposure %.4f", exposure);

        ImGui::Text("Tone mapping method:");
        TONE_MAP_METHOD toneMappingMethod = m_config.GetToneMapMethod();
        ImGui::RadioButton("None", (int *)&toneMappingMethod, TONE_MAP_METHOD_NONE);
        ImGui::SameLine();
        ImGui::RadioButton("Reinhard", (int *)&toneMappingMethod, TONE_MAP_METHOD_REINHARD);
        ImGui::SameLine();
        ImGui::RadioButton("Bruno Opsenica", (int *)&toneMappingMethod, TONE_MAP_METHOD_BRUNO_OPSENICA);
        ImGui::SameLine();
        ImGui::RadioButton("With exposure", (int *)&toneMappingMethod, TONE_MAP_METHOD_WITH_EXPOSURE);
        m_config.SetToneMapMethod(toneMappingMethod);

        bool enableGamma = m_config.IsGammaCorrectionEnabled();
        ImGui::Checkbox("Enable Gamma Correction", &enableGamma);
        m_config.ControlGammaCorrection(enableGamma);

        bool enableBloom = m_config.IsBloomEnabled();
        ImGui::Checkbox("Bloom Enabled", &enableBloom);
        m_config.ControlBloom(enableBloom);

        if (enableBloom)
        {
            float LuminanceThreshold = m_config.GetLuminanceThreshold();
            ImGui::SliderFloat("Luminance threshold", &LuminanceThreshold, 0.01f, 5.0f);
            m_config.SetLuminanceThreshold(LuminanceThreshold);

            float BloomStrength = m_config.GetBloomStrength();
            ImGui::SliderFloat("Bloom strength", &BloomStrength, 0.0f, 1.0f);
            m_config.SetBloomStrength(BloomStrength);

            float SigmaH = m_config.GetBloomSigmaH();
            ImGui::SliderFloat("Bloom Sigma H", &SigmaH, 0.0f, 100.0f);
            m_config.SetBloomSigmaH(SigmaH);

            float SigmaV = m_config.GetBloomSigmaV();
            ImGui::SliderFloat("Bloom Sigma V", &SigmaV, 0.0f, 100.0f);
            m_config.SetBloomSigmaV(SigmaV);
        }

        ImGui::TreePop();
    }
}

void CoreScene::TerrainGUI()
{
    if (m_config.GetTerrainGrid())
    {
        if (ImGui::TreeNode("Terrain"))
        {
            float maxHeight = m_config.GetTerrainMaxHeight();
            m_config.SetTerrainMaxHeight(maxHeight);

            float texTileSizeInWorldUnits = m_config.GetTerrainTexTileSizeInWorldUnits();
            ImGui::SliderFloat("Texture tile size in world units", &texTileSizeInWorldUnits, 1.0f, 200.0f);
            m_config.SetTerrainTexTileSizeInWorldUnits(texTileSizeInWorldUnits);

            float horizontalScale = m_config.GetTerrainHorizontalScale();
            ImGui::SliderFloat("Horizontal Scale", &horizontalScale, 1.0f, 10.0f);
            m_config.SetTerrainHorizontalScale(horizontalScale);

            float lowHeightPercent = m_config.GetTerrainLowHeightPercent();
            float highHeightPercent = m_config.GetTerrainHighHeightPercent();
            ImGui::SliderFloat("Low Height Percent", &lowHeightPercent, 0.0f, 1.0f);

            if (lowHeightPercent > highHeightPercent)
            {
                lowHeightPercent = highHeightPercent;
            }

            m_config.SetTerrainLowHeightPercent(lowHeightPercent);

            ImGui::SliderFloat("High Height Percent", &highHeightPercent, 0.0f, 1.0f);

            if (highHeightPercent < lowHeightPercent)
            {
                highHeightPercent = lowHeightPercent;
            }

            m_config.SetTerrainHighHeightPercent(highHeightPercent);

            glm::vec3 dir = m_config.GetTerrainSunlightDir();
            vec3 sunlightDir(dir.x, dir.y, dir.z);
            ImGui::gizmo3D("##Dir1", sunlightDir, 200.0f, imguiGizmo::modeDirection);
            glm::vec3 lightDirection = glm::vec3(sunlightDir.x, sunlightDir.y, sunlightDir.z);
            m_config.SetTerrainSunlightDir(lightDirection);

            float ambientFactor = m_config.GetTerrainAmbientFactor();
            ImGui::SliderFloat("Ambient Factor", &ambientFactor, 0.0f, 1.0f);
            m_config.SetTerrainAmbientFactor(ambientFactor);

            ImGui::Text("Terrain Render Mode");
            TERRAIN_RENDER_MODE renderMode = m_config.GetTerrainRenderMode();
            ImGui::RadioButton("Full", (int *)&renderMode, TERRAIN_RENDER_MODE_FULL);
            ImGui::SameLine();
            ImGui::RadioButton("Height", (int *)&renderMode, TERRAIN_RENDER_MODE_HEIGHT);
            ImGui::SameLine();
            ImGui::RadioButton("Lighting only", (int *)&renderMode, TERRAIN_RENDER_MODE_LIGHTING_ONLY);
            ImGui::SameLine();
            ImGui::RadioButton("Normals", (int *)&renderMode, TERRAIN_RENDER_MODE_NORMALS);
            ImGui::SameLine();
            ImGui::RadioButton("Simple Tex Coords", (int *)&renderMode, TERRAIN_RENDER_MODE_SIMPLE_TEXCOORDS);
            m_config.SetTerrainRenderMode(renderMode);

            ImGui::TreePop();
        }
    }
}

SceneObject *CoreScene::CreateSceneObject(Model *pModel)
{
    if (m_numSceneObjects >= NUM_SCENE_OBJECTS)
    {
        printf("%s:%d - out of scene objects space\n", __FILE__, __LINE__);
        exit(0);
    }

    CoreSceneObject *pCoreSceneObject = CreateSceneObjectInternal((CoreModel *)pModel);
    return pCoreSceneObject;
}

SceneObject *CoreScene::CreateSceneObject(const std::string &basicShape)
{
    CoreModel *pModel = (CoreModel *)m_pCoreRenderingSystem->GetModel(basicShape);
    CoreSceneObject *pCoreSceneObject = CreateSceneObjectInternal(pModel);
    return pCoreSceneObject;
}

CoreSceneObject *CoreScene::CreateSceneObjectInternal(CoreModel *pModel)
{
    m_sceneObjects[m_numSceneObjects].SetModel(pModel);

    CoreSceneObject *pCoreSceneObject = &(m_sceneObjects[m_numSceneObjects]);
    int id = m_numSceneObjects;
    pCoreSceneObject->SetID(id);
    pCoreSceneObject->SetName("SceneObject_" + std::to_string(id));
    pCoreSceneObject->InitMeshTransforms();

    m_numSceneObjects++;

    return pCoreSceneObject;
}

const std::vector<PointLight> &CoreScene::GetPointLights()
{
    if (m_pointLights.size() > 0)
    {
        return m_pointLights;
    }

    for (std::list<CoreSceneObject *>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++)
    {
        CoreSceneObject *pSceneObject = *it;

        const std::vector<PointLight> &pointLights = pSceneObject->GetModel()->GetPointLights();

        if (pointLights.size() > 0)
        {
            return pointLights;
        }
    }

    return m_pointLights;
}

const std::vector<SpotLight> &CoreScene::GetSpotLights()
{
    if (m_spotLights.size() > 0)
    {
        return m_spotLights;
    }

    for (std::list<CoreSceneObject *>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++)
    {
        CoreSceneObject *pSceneObject = *it;

        const std::vector<SpotLight> &spotLights = pSceneObject->GetModel()->GetSpotLights();

        if (spotLights.size() > 0)
        {
            return spotLights;
        }
    }

    return m_spotLights;
}

const std::vector<DirectionalLight> &CoreScene::GetDirLights()
{
    if (m_dirLights.size() > 0)
    {
        return m_dirLights;
    }

    for (std::list<CoreSceneObject *>::const_iterator it = m_renderList.begin(); it != m_renderList.end(); it++)
    {
        CoreSceneObject *pSceneObject = *it;

        const std::vector<DirectionalLight> &dirLights = pSceneObject->GetModel()->GetDirLights();

        if (dirLights.size() > 0)
        {
            return dirLights;
        }
    }

    return m_dirLights;
}