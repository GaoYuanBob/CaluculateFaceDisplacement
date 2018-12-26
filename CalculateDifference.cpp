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
	//// 1-14��ʾ14������(1 - 7���� - L, 8 - 14���� - R)��0Ŀǰû�ã����ܱ�ʾ����
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// ��Ӧ�����Ƿ��ѳ��ֹ�����һ�γ�����begin�ģ��ڶ�����end��

	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		//cout << "mesh_name = " << mesh_name << endl;
		// begin��UL3��end��UL3(1)����˳��������һ��Ҫ�ȵ���begin������ģ�ͣ��ٵ���end������ģ��
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// �����
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]) {
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// �����
		{
			if (!u_flag[idx + 7]) {
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// �����
		{
			if (!l_flag[idx]) {
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// �����
		{
			if (!l_flag[idx + 7]) {
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// ����ĸ�vector�е�ģ�͵����ƣ������ǲ��ǶԵ�
	/*for (auto i = 1; i < u_begin.size(); i++)
	if (!u_begin[i]) cout << "idx = " << i << ", ��ǰ���ѱ��γ�" << endl;
	else             cout << "idx = " << i << ", " << u_begin[i]->label().toStdString() << endl;
	for (auto i = 1; i < u_end.size(); i++)
	if (!u_end[i])   cout << "idx = " << i << ", ��ǰ���ѱ��γ�" << endl;
	else             cout << "idx = " << i << ", " << u_end[i]->label().toStdString() << endl;
	for (auto i = 1; i < l_begin.size(); i++)
	if (!l_begin[i]) cout << "idx = " << i << ", ��ǰ���ѱ��γ�" << endl;
	else             cout << "idx = " << i << ", " << l_begin[i]->label().toStdString() << endl;
	for (auto i = 1; i < l_end.size(); i++)
	if (!l_end[i])   cout << "idx = " << i << ", ��ǰ���ѱ��γ�" << endl;
	else             cout << "idx = " << i << ", " << l_end[i]->label().toStdString() << endl;*/
	// ����ĸ�vector�е�ģ�͵����ƣ������ǲ��ǶԵ�

	// �ö�Ӧģ�����ĵ��λ������ʾ���е��λ��
	vector<Point3f> ub_center_pos(15, Point3f(0, 0, 0)), ue_center_pos(15, Point3f(0, 0, 0)), lb_center_pos(15, Point3f(0, 0, 0)), le_center_pos(15, Point3f(0, 0, 0));
	// ��������ʼ14��������λ��
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		for (const auto& vt : u_begin[i]->cm.vert)
			ub_center_pos[i] += vt.P();
		ub_center_pos[i] /= u_begin[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));	// toStdString �е��ַ�����
		//printf(" ����Ϊ(%.2f, %.2f, %.2f)\n", ub_center_pos[i].X(), ub_center_pos[i].Y(), ub_center_pos[i].Z());
	}
	// ��������ʼ14��������λ��
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		for (const auto& vt : l_begin[i]->cm.vert)
			lb_center_pos[i] += vt.P();
		lb_center_pos[i] /= l_begin[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));	// toStdString �е��ַ�����
		//printf(" ����Ϊ(%.2f, %.2f, %.2f)\n", lb_center_pos[i].X(), lb_center_pos[i].Y(), lb_center_pos[i].Z());
	}
	// ����������ƽ���14��������λ��
	for (auto i = 1; i < 15; i++)
	{
		if (!u_end[i]) continue;
		for (const auto& vt : u_end[i]->cm.vert)
			ue_center_pos[i] += vt.P();
		ue_center_pos[i] /= u_end[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(u_end[i]->label().toLocal8Bit())); // toStdString �е��ַ�����
		//printf(" ����Ϊ(%.2f, %.2f, %.2f)\n", ue_center_pos[i].X(), ue_center_pos[i].Y(), ue_center_pos[i].Z());
	}
	// ����������ƽ���14��������λ��
	for (auto i = 1; i < 15; i++)
	{
		if (!l_end[i]) continue;
		for (const auto& vt : l_end[i]->cm.vert)
			le_center_pos[i] += vt.P();
		le_center_pos[i] /= l_end[i]->cm.vert.size();
		//cout << string(static_cast<const char *>(l_end[i]->label().toLocal8Bit()));	// toStdString �е��ַ�����
		//printf(" ����Ϊ(%.2f, %.2f, %.2f)\n", le_center_pos[i].X(), le_center_pos[i].Y(), le_center_pos[i].Z());
	}

	// ���������ݵ�λ�ƣ�����������д���
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		cout << string(static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));
		auto tmp_dis = ue_center_pos[i] - ub_center_pos[i];	// Point3f
		if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01)
			printf(" ���ĵ�û��λ��\n");
		else
			printf(" ���ĵ��λ��Ϊ (%.2f, %.2f, %.2f)\n", tmp_dis.X(), tmp_dis.Y(), tmp_dis.Z());
	}
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		cout << string(static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));
		auto tmp_dis = le_center_pos[i] - lb_center_pos[i];	// Point3f
		if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01)
			printf(" ���ĵ�û��λ��\n");
		else
			printf(" ���ĵ��λ��Ϊ (%.2f, %.2f, %.2f)\n", tmp_dis.X(), tmp_dis.Y(), tmp_dis.Z());
	}
}

void CalculateDiffPlugin::cal_average_displacement(MeshDocument& md) const
{
	//// 1-14��ʾ14������(1 - 7���� - L, 8 - 14���� - R)��0Ŀǰû�ã����ܱ�ʾ����
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// ��Ӧ�����Ƿ��ѳ��ֹ�����һ�γ�����begin�ģ��ڶ�����end��

	// printf("md.meshList.size() = %d\n", md.meshList.size());
	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		//cout << "mesh_name = " << mesh_name << endl;
		// begin��UL3��end��UL3(1)����˳��������һ��Ҫ�ȵ���begin������ģ�ͣ��ٵ���end������ģ��
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// �����
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]) {
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// �����
		{
			if (!u_flag[idx + 7]) {
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// �����
		{
			if (!l_flag[idx]) {
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// �����
		{
			if (!l_flag[idx + 7]) {
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// ��������������ݵ�ƽ��λ��
	for(auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		Point3f u_ave_dis = { 0, 0, 0 };
		for(auto j = 0; j < u_begin[i]->cm.vert.size(); j++)
			u_ave_dis += u_end[i]->cm.vert[j].P() - u_begin[i]->cm.vert[j].P();
		u_ave_dis /= u_begin[i]->cm.vert.size();
		if(fabs(u_ave_dis.X()) < 0.01 && fabs(u_ave_dis.Y()) < 0.01 && fabs(u_ave_dis.Z()) < 0.01)
			printf("%s û��λ��\n", static_cast<const char *>(u_begin[i]->label().toLocal8Bit()));
		else
			printf("%s λ��Ϊ (%.2f, %.2f, %.2f)\n", static_cast<const char *>(u_begin[i]->label().toLocal8Bit()), u_ave_dis.X(), u_ave_dis.Y(), u_ave_dis.Z());
	}
	// ��������������ݵ�ƽ��λ��
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		Point3f l_ave_dis = { 0, 0, 0 };
		for (auto j = 0; j < l_begin[i]->cm.vert.size(); j++)
			l_ave_dis += l_end[i]->cm.vert[j].P() - l_begin[i]->cm.vert[j].P();
		l_ave_dis /= l_begin[i]->cm.vert.size();
		if(fabs(l_ave_dis.X()) < 0.01 && fabs(l_ave_dis.Y()) < 0.01 && fabs(l_ave_dis.Z()) < 0.01)
			printf("%s û��λ��\n", static_cast<const char *>(l_begin[i]->label().toLocal8Bit()));
		else
			printf("%s λ��Ϊ (%.2f, %.2f, %.2f)\n", static_cast<const char *>(l_begin[i]->label().toLocal8Bit()), l_ave_dis.X(), l_ave_dis.Y(), l_ave_dis.Z());
	}
}

/// ��MeshLab�Ҳ�ģ���б��н�Ҫ������λ��faceģ��ѡ�У��������������md.mm()�Ϳ��Ի�ȡ���ģ����
/// ���������ϵ�ÿһ���㣬�ҵ�face������y��z������ö���Զ������㣬��Ϊ�����㣬����λ��
/// ������face�ϵĵ��Ѿ������λ�ƣ���ô����λ����ƽ�������ͬ��
void CalculateDiffPlugin::cal_face_displacement(MeshDocument &md) const
{
	auto f_model = md.mm();	// MeshModel*
	//// 1-14��ʾ14������(1 - 7���� - L, 8 - 14���� - R)��0Ŀǰû�ã����ܱ�ʾ����
	vector<MeshModel*> u_begin(15, nullptr), l_begin(15, nullptr), u_end(15, nullptr), l_end(15, nullptr);
	vector<bool> u_flag(15, false), l_flag(15, false);		// ��Ӧ�����Ƿ��ѳ��ֹ�����һ�γ�����begin�ģ��ڶ�����end��

	for (auto mmp : md.meshList)	// Mesh Model Pointer
	{
		auto q_mesh_name = mmp->label();	// QString
		auto mesh_name = string(static_cast<const char *>(q_mesh_name.toLocal8Bit()));	// string
		// cout << "mesh_name = " << mesh_name << endl;
		// begin��UL3��end��UL3(1)����˳��������һ��Ҫ�ȵ���begin������ģ�ͣ��ٵ���end������ģ��
		const auto idx = mesh_name.substr(2)[0] - '0';
		if (q_mesh_name.contains("UL"))	// �����
		{
			// printf("idx = %d\n", idx);
			if (!u_flag[idx]){
				u_flag[idx] = true;
				u_begin[idx] = mmp;
			}
			else u_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("UR"))	// �����
		{
			if (!u_flag[idx + 7]){
				u_flag[idx + 7] = true;
				u_begin[idx + 7] = mmp;
			}
			else u_end[idx + 7] = mmp;
		}
		else if (q_mesh_name.contains("LL"))	// �����
		{
			if (!l_flag[idx]){
				l_flag[idx] = true;
				l_begin[idx] = mmp;
			}
			else l_end[idx] = mmp;
		}
		else if (q_mesh_name.contains("LR"))	// �����
		{
			if (!l_flag[idx + 7]){
				l_flag[idx + 7] = true;
				l_begin[idx + 7] = mmp;
			}
			else l_end[idx + 7] = mmp;
		}
	}
	// Ϊ f_model �ϵ�ÿ�����¼λ�ƣ�������ֹ���¼��ÿ�μ���������2
	vector<Point3f> f_vert_dis(f_model->cm.vert.size(), Point3f{ 0, 0, 0 });
	// ���������ÿ�����λ�Ƽ��� f_vert_dis, i: ����ģ��, j: ģ���ϵĵ�
	for (auto i = 1; i < 15; i++)
	{
		if (!u_begin[i]) continue;
		// ���� i ������ÿ�����λ��
		for (auto j = 0; j < u_begin[i]->cm.vert.size(); j++)
		{
			Point3f tmp_dis = u_end[i]->cm.vert[j].P() - u_begin[i]->cm.vert[j].P();
			if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01) continue;
			// �� f_model ���ҵ��������Y��Z�����Ͼ��붼Ҫ�� 0.5 ���ڵĵ㣬Ϊ������dis
			for(auto vi = f_model->cm.vert.begin(); vi != f_model->cm.vert.end(); vi++)
			{
				if (fabs(vi->P().Y() - u_begin[i]->cm.vert[j].P().Y()) > 0.5 || fabs(vi->P().Z() - u_begin[i]->cm.vert[j].P().Z()) > 0.5)
					continue;
				const auto idx = vi - f_model->cm.vert.begin();
				f_vert_dis[idx] = (f_vert_dis[idx] == Point3f{ 0, 0, 0 }) ? tmp_dis : ((f_vert_dis[idx] + tmp_dis) / 2);
			} 
		}
	}
	// ���������ÿ�����λ�Ƽ��� f_vert_dis, i: ����ģ��, j: ģ���ϵĵ�
	for (auto i = 1; i < 15; i++)
	{
		if (!l_begin[i]) continue;
		// ���� i ������ÿ�����λ��
		for (auto j = 0; j < l_begin[i]->cm.vert.size(); j++)
		{
			Point3f tmp_dis = l_end[i]->cm.vert[j].P() - l_begin[i]->cm.vert[j].P();
			if (fabs(tmp_dis.X()) < 0.01 && fabs(tmp_dis.Y()) < 0.01 && fabs(tmp_dis.Z()) < 0.01) continue;
			// �� f_model ���ҵ��������Y��Z�����Ͼ��붼Ҫ�� 0.5 ���ڵĵ㣬Ϊ������dis
			for (auto vi = f_model->cm.vert.begin(); vi != f_model->cm.vert.end(); vi++)
			{
				if (fabs(vi->P().Y() - l_begin[i]->cm.vert[j].P().Y()) > 0.5 || fabs(vi->P().Z() - l_begin[i]->cm.vert[j].P().Z()) > 0.5)
					continue;
				const auto idx = vi - f_model->cm.vert.begin();
				f_vert_dis[idx] = (f_vert_dis[idx] == Point3f{ 0, 0, 0 }) ? tmp_dis : ((f_vert_dis[idx] + tmp_dis) / 2);
			}
		}
	}
// f_vert_dis �������, ����������ļ�
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
	printf("�沿ģ�͹��� %d ����, %d ���㱻��Ϊ�ƶ�\n", static_cast<int>(f_vert_dis.size()), cnt);
}

/// ��������ǰ������ģ�͵�λ�� - GY 2018.12.26
/// 1����Ӧ���λ�Ƶ�ƽ��������
/// 2��ѡ�������ģ����е������ƽ������λ��������
/// 3���ö�Ӧ�����ϵĶ�Ӧ�㣬���� face ģ�͵ĵ�λ�ƣ����õĺ�������ϸ����
// ��Ҫ�ȵ�������ǰ��� 14 + 14 ������ģ�ͣ��� UL1�����ٵ������ƽ������ 14 + 14 ������ģ�ͣ��� UL1(1)��
// ***** ����Ǽ��� face �ĵ��λ�ƣ�һ��Ҫ��meshList��ѡ����� face ��ģ��
bool CalculateDiffPlugin::StartEdit(MeshDocument &md, GLArea * gla, MLSceneGLSharedDataContext* )
{
	//printf("����1, 2 �� 3, 1: ͨ��ģ�����ĵ��λ��������ģ��λ�ƣ�2: ͨ��ģ�Ͷ�Ӧ���λ�Ƶ�ƽ�������㣬3: ����faceģ����ÿ�����λ��\n");
	//auto choice = 0;
	//cin >> choice;	// 1: ͨ��ģ�Ͷ�Ӧ���λ�Ƶ�ƽ��������ģ��λ�ƣ�2: ͨ��ģ�����ĵ��λ��������ģ��λ��
	//while (choice != 1 && choice != 2 && choice != 3) cin >> choice;
	//if (choice == 1)      cal_average_displacement(md);
	//else if(choice == 2)  cal_center_displacement(md);
	//else                  cal_face_displacement(md);

	cal_face_displacement(md);

////// ����update����, ���ģ��û�б仯�Ͳ���Ҫ��
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
////// ����update����

	return true;
}