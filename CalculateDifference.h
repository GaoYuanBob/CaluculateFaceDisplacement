#pragma once

#include <common/interfaces.h>

class CalculateDiffPlugin : public QObject, public MeshEditInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshEditInterface)
		
public:
	CalculateDiffPlugin();
    virtual ~CalculateDiffPlugin() = default;
	static QString Info();

    bool StartEdit(MeshDocument & , GLArea * , MLSceneGLSharedDataContext* ) override;
	void suggestedRenderingData(MeshModel &, MLRenderingData& ) override;

	void cal_center_displacement(MeshDocument& md) const;
	void cal_average_displacement(MeshDocument& md) const;
	void cal_face_displacement(MeshDocument &md) const;

// no implemention
	void EndEdit(MeshModel & , GLArea * , MLSceneGLSharedDataContext* ) override {};
    void Decorate(MeshModel & , GLArea * , QPainter * ) override {};
    void Decorate (MeshModel & , GLArea * ) override {};
    void mousePressEvent(QMouseEvent *, MeshModel &, GLArea * ) override {};
    void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea * ) override {};
    void mouseReleaseEvent(QMouseEvent * , MeshModel & , GLArea * ) override {};
	void keyReleaseEvent(QKeyEvent *, MeshModel &, GLArea *) override {};

private:
    QPoint cur;
	QFont qFont;
};
