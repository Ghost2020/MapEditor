
#include "stdafx.h"
#include "hierarchywindow.h"


using namespace graphic;

cHierarchyWindow::cHierarchyWindow()
	: cDockWindow("Hierarchy Window")
{
}

cHierarchyWindow::~cHierarchyWindow()
{
	Clear();
}


void cHierarchyWindow::OnUpdate(const float deltaSeconds)
{
}


void cHierarchyWindow::OnRender(const float deltaSeconds)
{
	if (ImGui::Button(" + Add Model"))
	{
		AddModel();
	}

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Always);
	if (ImGui::TreeNode((void*)0, "Hierarchy"))
	{
		ImGui::SameLine();
		const bool isExpandTree = ImGui::SmallButton("- Expand Tree");

		bool isClickItem = false;
		static int selectIdx = -1;
		for (auto &pnode : g_root.m_terrain.m_children) // parents node
		{
			if (isExpandTree)
			{
				ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
				ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Always);
			}

			if (ImGui::TreeNode((void*)pnode->m_id, pnode->m_name.utf8().c_str()))
			{
				for (auto *cnode : pnode->m_children)
				{
					const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow 
						| ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Leaf 
						| ImGuiTreeNodeFlags_NoTreePushOnOpen
						| ((cnode->m_id == selectIdx) ? ImGuiTreeNodeFlags_Selected : 0);

					ImGui::TreeNodeEx((void*)(intptr_t)cnode->m_id, node_flags,
						cnode->m_name.utf8().c_str());

					if (!isClickItem && ImGui::IsItemClicked())
					{
						isClickItem = true;
						selectIdx = cnode->m_id;
						g_root.m_selectModel = cnode;
						g_root.m_gizmo->SetControlNode(cnode);

						if (cTile *p = dynamic_cast<cTile*>(pnode))
							g_root.m_terrainEditWindow->m_gridEdit.SelectTile(p);
					}
				}
				ImGui::TreePop();
			}

			if (!isClickItem && ImGui::IsItemClicked())
			{
				isClickItem = true;
				selectIdx = pnode->m_id;
				g_root.m_selectModel = pnode;
				g_root.m_gizmo->SetControlNode(NULL);
				if (cTile *p = dynamic_cast<cTile*>(pnode))
					g_root.m_terrainEditWindow->m_gridEdit.SelectTile(p);
			}

		}

		ImGui::TreePop();
	}
}


void cHierarchyWindow::OnEventProc(const sf::Event &evt)
{
}


void cHierarchyWindow::Clear()
{
}


bool cHierarchyWindow::AddModel()
{
	Str16 ext = g_root.m_resWindow->m_selectPath.GetFileExt();
	if (((ext != ".x") && (ext != ".X"))
		&& ((ext != ".dae") && (ext != ".DAE"))
		&& ((ext != ".fbx") && (ext != ".FBX"))
		)
	{
		// error
		return false;
	}
	else
	{
		cRenderer &renderer = GetRenderer();

		cModel *model = new cModel();
		model->Create(renderer, common::GenerateId()
			, g_root.m_resWindow->m_selectPath.c_str(), true);

		AddModel(model);
	}

	return true;
}


bool cHierarchyWindow::AddModel(cNode *node)
{
	g_root.m_selectModel = node;
	g_root.m_gizmo->SetControlNode(node);
	g_root.m_mapView->m_pickMgr.Add(node);
	g_root.m_mapView->m_pickMgr.m_offset = { -10, -40 };

	assert(!g_root.m_terrain.m_tiles.empty());

	const Ray ray = g_root.m_camWorld.GetRay();
	Plane ground(Vector3(0, 1, 0), 0);
	const Vector3 pos = ground.Pick(ray.orig, ray.dir);
	node->m_transform.pos = pos;
	node->m_transform.pos.y = g_root.m_terrain.GetHeight(pos.x, pos.z);
	g_root.m_terrain.AddModel(node);

	// ���� �޽������� ���� ���� �ִٸ�, Scale, Rotation �� �����ϰ� �Ѵ�.
	// ������ ���� ���ٸ�, ���� ����� ����� ���� �������� �Ѵ�.
	// todo: ���� ���� �̸����� �˻��ϰ� �ִµ�, �޽������� �������� �˻��ؾ��Ѵ�.
	if (cModel *model = dynamic_cast<cModel*>(node))
	{
		vector<cNode*> candidates;
		g_root.m_terrain.FindNodeAll(node->m_name, candidates);

		cModel *mostNearModel = NULL;
		float nearLen = FLT_MAX;
		const Vector3 pos = model->GetWorldTransform().pos;
		for (auto &p : candidates)
		{
			cModel *mod = dynamic_cast<cModel*>(p);
			if (!mod)
				continue;
			if (node == p)
				continue;

			const float len = mod->GetWorldTransform().pos.LengthRoughly(pos);
			if (nearLen > len)
			{
				nearLen = len;
				mostNearModel = mod;
			}
		}

		if (mostNearModel)
		{
			node->m_transform.scale = mostNearModel->m_transform.scale;
			node->m_transform.rot = mostNearModel->m_transform.rot;
		}
	}

	return true;
}
