#include <meshlab/glarea.h>
#include "CalculateDifference.h"
#include <fstream>

using namespace std;
using namespace vcg;

CalculateDiffPlugin::CalculateDiffPlugin()
{
	qFont.setFamily("Helvetica");
	qFont.setPixelSize(12);
}

QString CalculateDiffPlugin::Info()
{
	return tr("Return detailed info about a picked face or vertex of the model.");
}

void CalculateDiffPlugin::suggestedRenderingData(MeshModel &m, MLRenderingData& dt)
{
	if (m.cm.VN() == 0)
		return;
	auto pr = MLRenderingData::PR_SOLID; // MLRenderingData::PRIMITIVE_MODALITY
	if (m.cm.FN() > 0)
		pr = MLRenderingData::PR_SOLID;

	MLRenderingData::RendAtts atts;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_FACENORMAL] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_FACECOLOR] = true;

	MLPerViewGLOptions opts;
	dt.get(opts);
	opts._sel_enabled = true;
	opts._face_sel = true;
	opts._vertex_sel = true;
	dt.set(opts);

	dt.set(pr, atts);
}

void CalculateDiffPlugin::cal_center_displacement(MeshDocument& md) const
{
	//// 1-14表示14颗牙齿(1 - 7是左 - L, 8 - 14是右 - R)，0目前没用，可能表示牙龈
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// 对应牙齿是否已出现过，第一次出现是begin的，第二次是end的

	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		//cout << "mesh_name = " << mesh_name << endl;
		// begin是UL3，end是UL3(1)，按顺序处理，所以一定要先导入begin的所有模型，再导入end的所有模型
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// 上颌左
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]) {
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// 上颌右
		{
			if (!u_flag[idx + 7]) {
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// 下颌左
		{
			if (!l_flag[idx]) {
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// 下颌右
		{
			if (!l_flag[idx + 7]) {
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// 输出四个vector中的模型的名称，看看是不是对的
	/*for (auto i = 1; i < u_begin.size(); i++)
	if (!u_begin[i]) cout << "idx = " << i << ", 当前牙已被拔除" << endl;
	else             cout << "idx = " << i << ", " << u_begin[i]->label().toStdString() << endl;
	for (auto i = 1; i < u_end.size(); i++)
	if (!u_end[i])   cout << "idx = " << i << ", 当前牙已被拔除" << endl;
	else             cout << "idx = " << i << ", " << u_end[i]->label().toStdString() << endl;
	for (auto i = 1; i < l_begin.size(); i++)
	if (!l_begin[i]) cout << "idx = " << i << ", 当前牙已被拔除" << endl;
	else             cout << "idx = " << i << ", " << l_begin[i]->label().toStdString() << endl;
	for (auto i = 1; i < l_end.size(); i++)
	if (!l_end[i])   cout << "idx = " << i << ", 当前牙已被拔除" << endl;
	else             cout << "idx = " << i << ", " << l_end[i]->label().toStdString() << endl;*/
	// 输出四个vector中的模型的名称，看看是不是对的

	// 用对应模型中心点的位移来表示所有点的位移
	vector<Point3f> ub_center_pos(15, Point3f(0, 0, 0)), ue_center_pos(15, Point3f(0, 0, 0)), lb_center_pos(15, Point3f(0, 0, 0)), le_center_pos(15, Point3f(0, 0, 0));
	// 计算上颌初始14颗牙中心位置
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		for (const auto& vt : u_begin[i]->cm.vert)
			ub_center_pos[i] += vt.P();
		ub_center_pos[i] /= u_begin[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));	// toStdString 有的字符不行
		//printf(" 中心为(%.2f, %.2f, %.2f)\n", ub_center_pos[i].X(), ub_center_pos[i].Y(), ub_center_pos[i].Z());
	}
	// 计算下颌初始14颗牙中心位置
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		for (const auto& vt : l_begin[i]->cm.vert)
			lb_center_pos[i] += vt.P();
		lb_center_pos[i] /= l_begin[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));	// toStdString 有的字符不行
		//printf(" 中心为(%.2f, %.2f, %.2f)\n", lb_center_pos[i].X(), lb_center_pos[i].Y(), lb_center_pos[i].Z());
	}
	// 计算上颌治疗结束14颗牙中心位置
	for (auto i = 1; i < 15; i++)
	{
		if (!u_end[i]) continue;
		for (const auto& vt : u_end[i]->cm.vert)
			ue_center_pos[i] += vt.P();
		ue_center_pos[i] /= u_end[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(u_end[i]->label().toLocal8Bit())); // toStdString 有的字符不行
		//printf(" 中心为(%.2f, %.2f, %.2f)\n", ue_center_pos[i].X(), ue_center_pos[i].Y(), ue_center_pos[i].Z());
	}
	// 计算下颌治疗结束14颗牙中心位置
	for (auto i = 1; i < 15; i++)
	{
		if (!l_end[i]) continue;
		for (const auto& vt : l_end[i]->cm.vert)
			le_center_pos[i] += vt.P();
		le_center_pos[i] /= l_end[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(l_end[i]->label().toLocal8Bit()));	// toStdString 有的字符不行
		//printf(" 中心为(%.2f, %.2f, %.2f)\n", le_center_pos[i].X(), le_center_pos[i].Y(), le_center_pos[i].Z());
	}

	// 将所有牙齿的位移，输出至命令行窗口
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		cout << string(static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));
		auto tmp_dis = ue_center_pos[i] - ub_center_pos[i];	// Point3f
		if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01)
			printf(" 中心点没有位移\n");
		else
			printf(" 中心点的位移为 (%.2f, %.2f, %.2f)\n", tmp_dis.X(), tmp_dis.Y(), tmp_dis.Z());
	}
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		cout << string(static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));
		auto tmp_dis = le_center_pos[i] - lb_center_pos[i];	// Point3f
		if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01)
			printf(" 中心点没有位移\n");
		else
			printf(" 中心点的位移为 (%.2f, %.2f, %.2f)\n", tmp_dis.X(), tmp_dis.Y(), tmp_dis.Z());
	}
}

void CalculateDiffPlugin::cal_average_displacement(MeshDocument& md) const
{
	//// 1-14表示14颗牙齿(1 - 7是左 - L, 8 - 14是右 - R)，0目前没用，可能表示牙龈
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// 对应牙齿是否已出现过，第一次出现是begin的，第二次是end的

	// printf("md.meshList.size() = %d\n", md.meshList.size());
	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		//cout << "mesh_name = " << mesh_name << endl;
		// begin是UL3，end是UL3(1)，按顺序处理，所以一定要先导入begin的所有模型，再导入end的所有模型
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// 上颌左
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]) {
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// 上颌右
		{
			if (!u_flag[idx + 7]) {
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// 下颌左
		{
			if (!l_flag[idx]) {
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// 下颌右
		{
			if (!l_flag[idx + 7]) {
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// 计算上颌所有牙齿的平均位移
	for(auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		Point3f u_ave_dis = { 0, 0, 0 };
		for(auto j = 0; j < u_begin[i]->cm.vert.size(); j++)
			u_ave_dis += u_end[i]->cm.vert[j].P() - u_begin[i]->cm.vert[j].P();
		u_ave_dis /= u_begin[i]->cm.vert.size();
		if(fabs(u_ave_dis.X()) < 0.01 && fabs(u_ave_dis.Y()) < 0.01 && fabs(u_ave_dis.Z()) < 0.01)
			printf("%s 没有位移\n", static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));
		else
			printf("%s 位移为 (%.2f, %.2f, %.2f)\n", static_cast<const char *>(u_begin[i]->label().toLocal8Bit()), u_ave_dis.X(), u_ave_dis.Y(), u_ave_dis.Z());
	}
	// 计算下颌所有牙齿的平均位移
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		Point3f l_ave_dis = { 0, 0, 0 };
		for (auto j = 0; j < l_begin[i]->cm.vert.size(); j++)
			l_ave_dis += l_end[i]->cm.vert[j].P() - l_begin[i]->cm.vert[j].P();
		l_ave_dis /= l_begin[i]->cm.vert.size();
		if(fabs(l_ave_dis.X()) < 0.01 && fabs(l_ave_dis.Y()) < 0.01 && fabs(l_ave_dis.Z()) < 0.01)
			printf("%s 没有位移\n", static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));
		else
			printf("%s 位移为 (%.2f, %.2f, %.2f)\n", static_cast<const char *>(l_begin[i]->label().toLocal8Bit()), l_ave_dis.X(), l_ave_dis.Y(), l_ave_dis.Z());
	}
}

/// 再MeshLab右侧模型列表中将要计算点的位移face模型选中（鼠标左键点击），md.mm()就可以获取这个模型了
/// 对于牙齿上的每一个点，找到face上与它y、z方向离得都不远的最近点，作为被动点，计算位移
/// 如果这个face上的点已经计算过位移，那么两次位移求平均，多次同理
void CalculateDiffPlugin::cal_face_displacement(MeshDocument &md) const
{
	auto f_model = md.mm();	// MeshModel*
	//// 1-14表示14颗牙齿(1 - 7是左 - L, 8 - 14是右 - R)，0目前没用，可能表示牙龈
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// 对应牙齿是否已出现过，第一次出现是begin的，第二次是end的

	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		// cout << "mesh_name = " << mesh_name << endl;
		// begin是UL3，end是UL3(1)，按顺序处理，所以一定要先导入begin的所有模型，再导入end的所有模型
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// 上颌左
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]){
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// 上颌右
		{
			if (!u_flag[idx + 7]){
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// 下颌左
		{
			if (!l_flag[idx]){
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// 下颌右
		{
			if (!l_flag[idx + 7]){
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// 为 f_model 上的每个点记录位移，如果出现过记录，每次加起来除以2
	vector<Point3f> f_vert_dis(f_model->cm.vert.size(), Point3f{ 0, 0, 0 });
	// 用上颌牙齿每个点的位移计算 f_vert_dis, i: 牙齿模型, j: 模型上的点
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		// 上颌第 i 颗牙齿每个点的位移
		for (auto j = 0; j < u_begin[i]->cm.vert.size(); j++)
		{
			Point3f tmp_dis = u_end[i]->cm.vert[j].P() - u_begin[i]->cm.vert[j].P();
			if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01) continue;
			// 在 f_model 上找到离这个点Y、Z方向上距离都要在 0.5 以内的点，为它设置dis
			for(auto vi = f_model->cm.vert.begin(); vi != f_model->cm.vert.end(); vi++)
			{
				if (fabs(vi->P().Y() - u_begin[i]->cm.vert[j].P().Y()) > 0.5 || fabs(vi->P().Z() - u_begin[i]->cm.vert[j].P().Z()) > 0.5)
					continue;
				const auto idx = vi - f_model->cm.vert.begin();
				f_vert_dis[idx] = (f_vert_dis[idx] == Point3f{ 0, 0, 0 }) ? tmp_dis : ((f_vert_dis[idx] + tmp_dis) / 2);
			} 
		}
	}
	// 用下颌牙齿每个点的位移计算 f_vert_dis, i: 牙齿模型, j: 模型上的点
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		// 上颌第 i 颗牙齿每个点的位移
		for (auto j = 0; j < l_begin[i]->cm.vert.size(); j++)
		{
			Point3f tmp_dis = l_end[i]->cm.vert[j].P() - l_begin[i]->cm.vert[j].P();
			if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01) continue;
			// 在 f_model 上找到离这个点Y、Z方向上距离都要在 0.5 以内的点，为它设置dis
			for (auto vi = f_model->cm.vert.begin(); vi != f_model->cm.vert.end(); vi++)
			{
				if (fabs(vi->P().Y() - l_begin[i]->cm.vert[j].P().Y()) > 0.5 || fabs(vi->P().Z() - l_begin[i]->cm.vert[j].P().Z()) > 0.5)
					continue;
				const auto idx = vi - f_model->cm.vert.begin();
				f_vert_dis[idx] = (f_vert_dis[idx] == Point3f{ 0, 0, 0 }) ? tmp_dis : ((f_vert_dis[idx] + tmp_dis) / 2);
			}
		}
	}
// f_vert_dis 计算完成, 可以输出至文件
	ofstream output;
	output.open(R"(C:\Users\Administrator\Desktop\face_vert_newPos.txt)");
	auto cnt = 0;
	for (auto i = 0; i < f_vert_dis.size(); i++)
		if (f_vert_dis[i] != Point3f{ 0, 0, 0 })
		{
			++cnt;
			f_model->cm.vert[i].C() = Color4b::Red;
			Point3f p = f_model->cm.vert[i].P() + f_vert_dis[i];
			output << i << " " << p.X() << " " << p.Y() << " " << p.Z() << endl;
			//printf("f_vert_dis[%d] = (%.2f, %.2f, %.2f)\n", i, f_vert_dis[i].X(), f_vert_dis[i].Y(), f_vert_dis[i].Z());
		}
	printf("面部模型共有 %d 个点, %d 个点被设为移动\n", static_cast<int>(f_vert_dis.size()), cnt);
}

/// 计算治疗前后牙齿模型的位移 - GY 2018.12.26
/// 1、对应点的位移的平均来计算
/// 2、选择用中心（所有点坐标的平均）的位移来计算
/// 3、用对应牙齿上的对应点，驱动 face 模型的点位移，调用的函数会详细解释
// 需要先导入治疗前最多 14 + 14 颗牙齿模型（如 UL1），再导入治疗结束最多 14 + 14 颗牙齿模型（如 UL1(1)）
// ***** 如果是计算 face 的点的位移，一定要在meshList里选中这个 face 的模型
bool CalculateDiffPlugin::StartEdit(MeshDocument &md, GLArea * gla, MLSceneGLSharedDataContext* )
{
	//printf("输入1, 2 或 3, 1: 通过模型中心点的位移来计算模型位移，2: 通过模型对应点的位移的平均来计算，3: 计算face模型上每个点的位移\n");
	//auto choice = 0;
	//cin >> choice;	// 1: 通过模型对应点的位移的平均来计算模型位移，2: 通过模型中心点的位移来计算模型位移
	//while (choice != 1 && choice != 2 && choice != 3) cin >> choice;
	//if (choice == 1)      cal_average_displacement(md);
	//else if(choice == 2)  cal_center_displacement(md);
	//else                  cal_face_displacement(md);

	cal_face_displacement(md);

////// 所有update操作, 如果模型没有变化就不需要用
	for (auto mm : md.meshList) // MeshModel*
	{
		gla->mvc()->sharedDataContext()->meshInserted(mm->id());
		MLRenderingData dt;
		gla->mvc()->sharedDataContext()->getRenderInfoPerMeshView(mm->id(), gla->context(), dt);
		suggestedRenderingData(*mm, dt);
		MLPoliciesStandAloneFunctions::disableRedundatRenderingDataAccordingToPriorities(dt);
		gla->mvc()->sharedDataContext()->setRenderingDataPerMeshView(mm->id(), gla->context(), dt);
		gla->update();
	}
////// 所有update操作

	return true;
}