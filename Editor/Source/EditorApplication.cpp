#include "EditorApplication.h"
#include <imgui/imgui.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

namespace SockEngine {

EditorApplication::EditorApplication()
    : Application("Sock Engine")
{
    // Initialize renderer
    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->Initialize();
    
    // Create a default scene
    m_ActiveScene = std::make_unique<Scene>("New Scene");
    
    // Set skybox
    std::vector<std::string> skyboxFaces {
        "../Assets/Textures/SkyboxDay/right.bmp",
        "../Assets/Textures/SkyboxDay/left.bmp",
        "../Assets/Textures/SkyboxDay/top.bmp",
        "../Assets/Textures/SkyboxDay/bottom.bmp",
        "../Assets/Textures/SkyboxDay/front.bmp",
        "../Assets/Textures/SkyboxDay/back.bmp"
    };
    // Load skybox textures
    m_Renderer->LoadSkybox(skyboxFaces);
    m_SkyboxEnabled = m_Renderer->IsSkyboxEnabled();
    
    // Regular environment model
    m_ActiveScene->LoadModel("../Assets/Models/sponza/sponza/Sponza.gltf");

    // Animated character model
    m_ActiveScene->LoadModel("../Assets/Models/mannequin/mannequin.fbx",
                            "../Assets/Models/mannequin/mannequin.fbx",
                            glm::vec3(0, 1315, -300), glm::vec3(1, 1, 1));
}

EditorApplication::~EditorApplication() {
    if (m_Renderer) {
        m_Renderer->Shutdown();
    }
}

void EditorApplication::OnUpdate(float deltaTime) {
    // Handle right mouse button state for camera
    static bool wasRightMouseButtonDown = false;
    bool isRightMouseButtonDown = m_Input.GetMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT);

    // Check if we should start/stop mouse capture
    if (m_ViewportFocused && m_ViewportBoundsValid && GetWindow().IsFocused()) {
        if (isRightMouseButtonDown && !wasRightMouseButtonDown) {
            // Start mouse capture when right-clicking in viewport
            m_Input.StartMouseCapture(GetWindow().GetNativeWindow(), m_ViewportMin, m_ViewportMax);
        }
    }
    
    if (!isRightMouseButtonDown && wasRightMouseButtonDown) {
        // Stop mouse capture when releasing right mouse button
        m_Input.EndMouseCapture(GetWindow().GetNativeWindow());
    }
    
    // Also stop capture if we lose focus
    if (!GetWindow().IsFocused() && m_Input.IsMouseCaptured()) {
        m_Input.EndMouseCapture(GetWindow().GetNativeWindow());
    }

    // Handle camera movement when mouse is captured
    if (m_Input.IsMouseCaptured() && isRightMouseButtonDown) {
        Camera& camera = m_ActiveScene->GetCamera();
        
        // Keyboard movement
        if (m_Input.GetKeyHeld(GLFW_KEY_W)) {
            camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (m_Input.GetKeyHeld(GLFW_KEY_A)) {
            camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (m_Input.GetKeyHeld(GLFW_KEY_S)) {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (m_Input.GetKeyHeld(GLFW_KEY_D)) {
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }

        // Mouse look
        glm::vec2 mouseDelta = m_Input.GetMouseDelta();
        if (glm::length(mouseDelta) > 0.01f) {
            float sensitivity = 0.1f;
            camera.ProcessMouseMovement(mouseDelta.x * sensitivity, -mouseDelta.y * sensitivity);
        }
    }
    
    // Scroll wheel zoom (works when viewport is hovered, even without mouse capture)
    if ((m_ViewportFocused || m_ViewportHovered) && !m_Input.IsMouseCaptured()) {
        glm::vec2 scrollDelta = m_Input.GetMouseScroll();
        if (scrollDelta.y != 0.0f) {
            Camera& camera = m_ActiveScene->GetCamera();
            camera.ProcessMouseScroll(scrollDelta.y);
        }
    }
    
    wasRightMouseButtonDown = isRightMouseButtonDown;
    
    // Update scene
    m_ActiveScene->OnUpdate(deltaTime);
}

void EditorApplication::OnRender() {
    // Apply debug settings to renderer
    m_Renderer->EnableDebugNormals(m_DebugNormals);
    m_Renderer->EnableDebugSpecular(m_DebugSpecular);
    
    // Render scene
    m_Renderer->RenderScene(*m_ActiveScene, m_ActiveScene->GetCamera());
}

void EditorApplication::OnImGuiRender() {
    // Set up the dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("Main Dockspace", nullptr, window_flags);
    ImGui::PopStyleColor(1);
    ImGuiID dockspace_id = ImGui::GetID("Main Dockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    DrawMenuBar();
    ImGui::End();

    // Draw the About window if the flag is set
    if (m_ShowAboutWindow) {
        DrawAboutWindow();
    }
    
    // Draw editor windows
    DrawViewport();
    DrawSceneHierarchy();
    DrawInspector();
    DrawDebugPanel();
    DrawOutputLog();
}

void EditorApplication::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        std::vector<Menu> menus = {
            { "File", {
                { "New Scene", "CTRL+N", []() {} },
                { "Open Scene", "CTRL+O", []() {} },
                { "Save Scene", "CTRL+S", []() {} },
                { "Save Scene As...", "CTRL+ALT+S", []() {} },
                { "Save All", "CTRL+SHIFT+S", []() {} },
                { "Exit", nullptr, [this]() { Close(); } }
            }},
            { "Edit", {
                { "Undo", "CTRL+Z", []() {} },
                { "Redo", "CTRL+Y", []() {} },
                { "Cut", "CTRL+X", []() {} },
                { "Copy", "CTRL+C", []() {} },
                { "Paste", "CTRL+V", []() {} },
                { "Settings", nullptr, []() {} }
            }},
            { "View", {
                { "Scene Hierarchy", nullptr, []() {} },
                { "Inspector", nullptr, []() {} },
                { "Viewport", nullptr, []() {} }
            }},
            { "Help", {
                { "About", nullptr, [this]() { ShowAboutWindow(); } },
            }}
        };

        // Render all menus
        for (const auto& menu : menus) {
            if (ImGui::BeginMenu(menu.name)) {
                for (const auto& item : menu.items) {
                    if (ImGui::MenuItem(item.name, item.shortcut)) {
                        item.action();
                    }
                }
                ImGui::EndMenu();
            }
        }
        
        ImGui::EndMenuBar();
    }
}

void EditorApplication::ShowAboutWindow() {
    // Set show_about_window flag to true
    m_ShowAboutWindow = true;
}

void EditorApplication::DrawAboutWindow() {
    // Calculate center position for the window
    ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    // Load logo
    GLuint logo = 0;
    ImVec2 logoSize = ImVec2(0, 0);
    int width, height, channels;
    unsigned char* data = SOIL_load_image("../Assets/Branding/sockenginelogo.png", &width, &height, &channels, 0);
    if (data)
    {
        glGenTextures(1, &logo);
        glBindTexture(GL_TEXTURE_2D, logo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        logoSize = ImVec2((float)width, (float)height);
        SOIL_free_image_data(data);
    }

    float displayHeight = 400.0f;
    float aspectRatio = logoSize.x / logoSize.y;
    ImVec2 displaySize(displayHeight * aspectRatio, displayHeight);
    
    // Create the window
    ImGui::Begin("##", &m_ShowAboutWindow, window_flags);
    ImGui::PopStyleVar(1);
    ImGui::Image((void*)(intptr_t)logo, displaySize);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Sock Engine");
    ImGui::Text("Version 0.1.0");
    ImGui::Separator();
    ImGui::Text("© 2025 Sock Games");
    ImGui::End();
    
}

void EditorApplication::DrawViewport() {
    ImGui::Begin("Viewport");
    
    // Store viewport focus/hover state
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    
    // Get the size of the viewport
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    
    // Update viewport bounds for mouse capture
    m_ViewportMin = {viewportPos.x, viewportPos.y};
    m_ViewportMax = {viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y};
    m_ViewportBoundsValid = (viewportSize.x > 10 && viewportSize.y > 10);
    
    // Display the rendered scene in the viewport
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage(
        (void*)m_Renderer->GetFramebufferTexture(),
        ImVec2(viewportPos.x, viewportPos.y),
        ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );
    
    ImGui::End();
}

void EditorApplication::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");

    // Display the root entity (the scene) as a header item
    Entity rootEntity = m_ActiveScene->GetRootEntity();
    
    // Create a collapsible header for the scene
    std::string headerText = "Scene Root (" + m_ActiveScene->GetName() + ")";
    bool headerOpen = ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    
    // Make the scene root selectable by clicking on the name
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_ActiveScene->SetSelectedEntity(rootEntity);
    }

    // Handle drag-drop for reparenting to the scene root
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            Entity* droppedEntity = (Entity*)payload->Data;
            if (*droppedEntity && *droppedEntity != rootEntity) {
                m_ActiveScene->UpdateRelationship(*droppedEntity, rootEntity);
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    if (headerOpen) {
        // Draw all root entities and their children
        for (auto entity : m_ActiveScene->GetRootEntities()) {
            DrawEntityNode(entity);
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && 
        !ImGui::IsAnyItemHovered())
    {
        m_ActiveScene->SetSelectedEntity(Entity());
    }

    // Right-click on empty space in the window
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && 
        !ImGui::IsAnyItemHovered())
    {
        ImGui::OpenPopup("SceneHierarchyContextMenu");
    }
    
    if (ImGui::BeginPopup("SceneHierarchyContextMenu")) {
        if (ImGui::MenuItem("Create Empty Object")) {
            Entity entity = m_ActiveScene->CreateEntity("Empty Object");
            m_ActiveScene->SetSelectedEntity(entity);
        }
        ImGui::EndPopup();
    }
    
    // Handle scheduled entity deletion
    if (m_EntityToDelete) {
        m_ActiveScene->DestroyEntity(m_EntityToDelete);
        m_EntityToDelete = Entity();
    }
    
    ImGui::End();
}

void EditorApplication::DrawEntityNode(Entity entity) {
    if (!entity) {
        return;
    }

    // Skip if this is the scene root entity (it's handled separately)
    if (entity == m_ActiveScene->GetRootEntity()) {
        return;
    }
    
    // Create unique identifiers for this entity
    std::string nodeId = "Entity_" + std::to_string(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
    std::string popupId = "ContextMenu_" + std::to_string(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
    
    // Set up tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    // Add selected flag if this entity is selected
    if (entity == m_ActiveScene->GetSelectedEntity()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    // If no children, make it a leaf node
    bool hasChildren = entity.HasComponent<RelationshipComponent>() && 
                       !entity.GetComponent<RelationshipComponent>().children.empty();
                       
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    
    // Begin the tree node
    bool opened = ImGui::TreeNodeEx(nodeId.c_str(), flags, "%s", entity.GetName().c_str());
    
    // Handle selection when clicked
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_ActiveScene->SetSelectedEntity(entity);
    }
    
    // Explicitly check for right-click on the item to open the popup
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(popupId.c_str());
    }
    
    // Draw the context menu
    if (ImGui::BeginPopup(popupId.c_str())) {
        if (ImGui::MenuItem("Create Empty Child")) {
            Entity childEntity = m_ActiveScene->CreateEntity("Empty Object", entity);
            m_ActiveScene->SetSelectedEntity(childEntity);
            ImGui::SetNextItemOpen(true);
        }
        
        if (ImGui::MenuItem("Duplicate")) {
            Entity duplicate = m_ActiveScene->DuplicateEntity(entity);
            if (duplicate) {
                m_ActiveScene->SetSelectedEntity(duplicate);
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Delete Object")) {
            m_EntityToDelete = entity;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Handle drag-drop for reparenting
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(Entity));
        ImGui::Text("Moving %s", entity.GetName().c_str());
        ImGui::EndDragDropSource();
    }
    
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            Entity* droppedEntity = (Entity*)payload->Data;
            if (*droppedEntity != entity && *droppedEntity) {
                m_ActiveScene->UpdateRelationship(*droppedEntity, entity);
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    // Draw children if the node is open
    if (opened) {
        if (entity.HasComponent<RelationshipComponent>()) {
            auto& relationship = entity.GetComponent<RelationshipComponent>();
            
            for (auto childHandle : relationship.children) {
                Entity childEntity(childHandle, &m_ActiveScene->GetSceneRegistry());
                if (childEntity) {
                    DrawEntityNode(childEntity);
                }
            }
        }
        ImGui::TreePop();
    }
}

void EditorApplication::DrawInspector() {
    ImGui::Begin("Inspector");
    
    Entity selectedEntity = m_ActiveScene->GetSelectedEntity();
    if (!selectedEntity) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }
    
    auto& registry = m_ActiveScene->GetNativeRegistry();
    auto entityHandle = static_cast<entt::entity>(selectedEntity);

    // Special handling for scene root entity
    if (selectedEntity == m_ActiveScene->GetRootEntity()) {
        ImGui::Text("Scene Properties");
        
        char nameBuffer[256] = "\0";
        strcpy(nameBuffer, m_ActiveScene->GetName().c_str());
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        if (ImGui::InputText("Scene Name", nameBuffer, sizeof(nameBuffer))) {
            m_ActiveScene->SetName(nameBuffer);
        }
        
        // Any scene-specific settings could go here
        ImGui::Separator();
        
        // No need to show components for the scene root
        ImGui::End();
        return;
    }
    
    // Entity name and active state
    char nameBuffer[256] = "\0";
    strcpy(nameBuffer, selectedEntity.GetName().c_str());
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        selectedEntity.SetName(nameBuffer);
    }
    
    bool isActive = registry.all_of<ActiveComponent>(entityHandle) ? 
                    registry.get<ActiveComponent>(entityHandle).active : true;
                    
    if (ImGui::Checkbox("Active", &isActive)) {
        if (registry.all_of<ActiveComponent>(entityHandle)) {
            registry.get<ActiveComponent>(entityHandle).active = isActive;
        }
    }
    
    ImGui::Separator();
    
    // Display existing components
    DrawComponents(selectedEntity);

    ImGui::Separator();
    
    // Add component button
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }
    
    if (ImGui::BeginPopup("AddComponentPopup")) {
        DrawAddComponentPopup(selectedEntity);
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void EditorApplication::DrawComponents(Entity entity) {
    if (!entity) return;
    
    auto& registry = m_ActiveScene->GetNativeRegistry();
    auto entityHandle = static_cast<entt::entity>(entity);
    
    // Draw Transform component (should be present on all entities except scene root)
    if (registry.all_of<TransformComponent>(entityHandle)) {
        DrawTransformComponent(entity);
    }
    
    // Draw Model component if present
    if (registry.all_of<ModelComponent>(entityHandle)) {
        DrawModelComponent(entity);
    }

    // Draw Animator component if present
    if (registry.all_of<AnimatorComponent>(entityHandle)) {
        DrawAnimatorComponent(entity);
    }
}

void EditorApplication::DrawTransformComponent(Entity entity) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& registry = m_ActiveScene->GetNativeRegistry();
        auto entityHandle = static_cast<entt::entity>(entity);
        auto& transformComponent = registry.get<TransformComponent>(entityHandle);
        
        // Position
        glm::vec3 position = transformComponent.localPosition;
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
            transformComponent.localPosition = position;
            transformComponent.localMatrixDirty = true;
            transformComponent.worldMatrixDirty = true;
            
            // Mark all children's world matrices as dirty
            entity.MarkChildrenWorldMatrixDirty();
        }
        
        // Rotation
        glm::vec3 editorRotation = transformComponent.localRotationDegrees;
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(editorRotation), 0.1f)) {
            transformComponent.localRotationDegrees = editorRotation;
            
            // Convert the editor rotation to a quaternion
            glm::quat quatX = glm::angleAxis(glm::radians(editorRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat quatY = glm::angleAxis(glm::radians(editorRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat quatZ = glm::angleAxis(glm::radians(editorRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            
            // Combined rotation (order: Z then Y then X)
            transformComponent.localRotation = quatX * quatY * quatZ;
            transformComponent.localRotation = glm::normalize(transformComponent.localRotation);
            
            transformComponent.localMatrixDirty = true;
            transformComponent.worldMatrixDirty = true;
            
            // Mark all children's world matrices as dirty
            entity.MarkChildrenWorldMatrixDirty();
        }
        
        // Scale
        glm::vec3 scale = transformComponent.localScale;
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f)) {
            transformComponent.localScale = scale;
            transformComponent.localMatrixDirty = true;
            transformComponent.worldMatrixDirty = true;
            
            // Mark all children's world matrices as dirty
            entity.MarkChildrenWorldMatrixDirty();
        }
    }
}

void EditorApplication::DrawModelComponent(Entity entity) {
    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& registry = m_ActiveScene->GetNativeRegistry();
        auto entityHandle = static_cast<entt::entity>(entity);
        auto& modelComponent = registry.get<ModelComponent>(entityHandle);
        
        // Display model path
        ImGui::Text("Model: %s", modelComponent.modelPath.c_str());
        
        // Rendering options
        bool castShadows = modelComponent.castShadows;
        if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
            modelComponent.castShadows = castShadows;
        }
        
        bool receiveShadows = modelComponent.receiveShadows;
        if (ImGui::Checkbox("Receive Shadows", &receiveShadows)) {
            modelComponent.receiveShadows = receiveShadows;
        }
        
        // Load model button
        if (ImGui::Button("Load Model")) {
            // Currently, there is no file system, so a popup will be shown for now
            ImGui::OpenPopup("ModelLoadNotSupported");
        }
        
        if (ImGui::BeginPopup("ModelLoadNotSupported")) {
            ImGui::Text("Sorry, not implemented yet :(");
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void EditorApplication::DrawAnimatorComponent(Entity entity) {
    if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& registry = m_ActiveScene->GetNativeRegistry();
        auto entityHandle = static_cast<entt::entity>(entity);
        auto& animatorComponent = registry.get<AnimatorComponent>(entityHandle);
        
        // Initialize animator if needed
        if (!animatorComponent.animator && registry.all_of<ModelComponent>(entityHandle)) {
            auto& modelComponent = registry.get<ModelComponent>(entityHandle);
            if (modelComponent.model && !modelComponent.modelPath.empty()) {
                // Try to initialize with the model file (assuming it contains animations)
                animatorComponent.Initialize(modelComponent.model, modelComponent.modelPath);
            }
        }
        
        // Show current animation info
        if (animatorComponent.currentAnimation) {
            ImGui::Text("Current Animation: %s", animatorComponent.currentAnimationName.c_str());
            ImGui::Text("Duration: %.0f ticks", animatorComponent.GetDuration());
            ImGui::Text("Current Tick: %.0f", animatorComponent.GetCurrentTime());
            
            // Progress bar
            float progress = animatorComponent.GetDuration() > 0.0f ? 
                           animatorComponent.GetCurrentTime() / animatorComponent.GetDuration() : 0.0f;
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
            
            // Play/Pause button
            if (animatorComponent.IsPlaying()) {
                if (ImGui::Button("Pause")) {
                    animatorComponent.Pause();
                }
            } else {
                if (ImGui::Button("Play")) {
                    animatorComponent.Play();
                }
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                animatorComponent.Stop();
            }
            
            // Looping checkbox
            bool isLooping = animatorComponent.IsLooping();
            if (ImGui::Checkbox("Loop", &isLooping)) {
                animatorComponent.SetLooping(isLooping);
            }
            
            // Playback speed
            float speed = animatorComponent.GetPlaybackSpeed();
            if (ImGui::SliderFloat("Speed", &speed, 0.0f, 3.0f, "%.2fx")) {
                animatorComponent.SetPlaybackSpeed(speed);
            }
            
            // Animation selection
            if (animatorComponent.animations.size() > 1) {
                ImGui::Text("Available Animations:");
                
                for (const auto& [name, animation] : animatorComponent.animations) {
                    bool isSelected = (name == animatorComponent.currentAnimationName);
                    
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        if (!isSelected) {
                            animatorComponent.PlayAnimation(name);
                        }
                    }
                }
            }
            
            // Animation loading
            if (ImGui::Button("Load Animation File")) {
                auto& modelComponent = registry.get<ModelComponent>(entityHandle);
                if (modelComponent.model) {
                    // Currently, there is no file system, so a popup will be shown for now
                    ImGui::OpenPopup("AnimationLoadNotSupported");
                }
            }
            
            if (ImGui::BeginPopup("AnimationLoadNotSupported")) {
                ImGui::Text("Sorry, not implemented yet :(");
                if (ImGui::Button("Close")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
        } else {
            ImGui::Text("No animation loaded");
            
            // Try to auto-initialize if we have a model
            if (registry.all_of<ModelComponent>(entityHandle)) {
                auto& modelComponent = registry.get<ModelComponent>(entityHandle);
                
                if (ImGui::Button("Initialize with Model")) {
                    if (modelComponent.model && !modelComponent.modelPath.empty()) {
                        animatorComponent.Initialize(modelComponent.model, modelComponent.modelPath);
                    }
                }
                
                ImGui::TextWrapped("Tip: Make sure your model file contains animation data, or load a separate animation file.");
            } else {
                ImGui::TextWrapped("Add a Model component first, then initialize the animator.");
            }
        }
    }
}

void EditorApplication::DrawAddComponentPopup(Entity entity) {
    if (!entity) {
        return;
    }
    
    auto& registry = m_ActiveScene->GetNativeRegistry();
    auto entityHandle = static_cast<entt::entity>(entity);
    
    if (ImGui::MenuItem("Model")) {
        if (!registry.all_of<ModelComponent>(entityHandle)) {
            auto& model = registry.emplace<ModelComponent>(entityHandle);
            ImGui::CloseCurrentPopup();
        }
        else {
            ImGui::CloseCurrentPopup();
        }
    }

    if (ImGui::MenuItem("Animator")) {
        if (!registry.all_of<AnimatorComponent>(entityHandle)) {
            auto& animator = registry.emplace<AnimatorComponent>(entityHandle);
            ImGui::CloseCurrentPopup();
        }
        else {
            ImGui::CloseCurrentPopup();
        }
    }
}

void EditorApplication::DrawDebugPanel() {
    ImGui::Begin("Debug");
    
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::Text("Frame Time: %.3f ms", m_FrameTime);
    
    ImGui::Spacing();
    
    // VSync
    bool enabled = m_Window->IsVSync();
    if (ImGui::Checkbox("VSync", &enabled)) {
        m_Window->SetVSync(enabled);
    }
    
    ImGui::Separator();

    // Render Resolution Settings
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Current: %dx%d", m_Renderer->GetRenderWidth(), m_Renderer->GetRenderHeight());
        
        if (ImGui::BeginCombo("Render Resolution", m_ResolutionOptions[m_SelectedResolutionIndex].name)) {
            for (int i = 0; i < m_ResolutionOptions.size(); i++) {
                bool isSelected = (m_SelectedResolutionIndex == i);
                if (ImGui::Selectable(m_ResolutionOptions[i].name, isSelected)) {
                    m_SelectedResolutionIndex = i;
                    
                    auto& option = m_ResolutionOptions[i];
                    m_Renderer->SetRenderResolution(option.width, option.height);
                }
                
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::Separator();
    
    // Debug visualization options
    if (ImGui::CollapsingHeader("Debug Visualizations", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Debug Normals", &m_DebugNormals);
        if (m_DebugNormals) {
            m_DebugSpecular = false;
        }
    
        ImGui::Checkbox("Debug Specular", &m_DebugSpecular);
        if (m_DebugSpecular) {
            m_DebugNormals = false;
        }
        
        if (ImGui::Checkbox("Enable Skybox", &m_SkyboxEnabled)) {
            m_Renderer->EnableSkybox(m_SkyboxEnabled);
        }
    }
    
    ImGui::Separator();
    
    // Camera settings
    Camera& camera = m_ActiveScene->GetCamera();
    
    if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Movement Speed", &camera.MovementSpeed, 100.0f, 8000.0f, "%.1f");
    }

    ImGui::Separator();
    
    // Input debug info
    if (ImGui::CollapsingHeader("Input Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Mouse position and delta
        glm::vec2 mousePos = m_Input.GetMousePosition();
        glm::vec2 mouseDelta = m_Input.GetMouseDelta();
        ImGui::Text("Mouse Position: %.1f, %.1f", mousePos.x, mousePos.y);
        ImGui::Text("Mouse Delta: %.1f, %.1f", mouseDelta.x, mouseDelta.y);
    
        // Mouse buttons state
        ImGui::Text("Right Mouse Button: %s", 
                    m_Input.GetMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) ? "Down" : "Up");
    
        // Cursor lock state
        ImGui::Text("Cursor Locked: %s", m_Input.IsMouseCaptured() ? "Yes" : "No");
    }
    
    ImGui::End();
}

void EditorApplication::DrawOutputLog() {
    ImGui::Begin("Output Log");
    // Currently, there is no logging system, so the window will just be empty for now.
    ImGui::End();
}

}
