#include "pipeline.h"

const glm::mat4 &Pipeline::GetProjTrans()
{
    m_ProjTransformation = glm::perspectiveFov(glm::radians(m_persProjInfo.FOV),
                                               m_persProjInfo.Width,
                                               m_persProjInfo.Height,
                                               m_persProjInfo.zNear,
                                               m_persProjInfo.zFar);
    return m_ProjTransformation;
}

const glm::mat4 &Pipeline::GetVPTrans()
{
    GetViewTrans();
    GetProjTrans();

    m_VPtransformation = m_ProjTransformation * m_Vtransformation;
    return m_VPtransformation;
}

const glm::mat4 &Pipeline::GetWorldTrans()
{
    glm::mat4 ScaleTrans = glm::scale(glm::mat4(1.0f), m_scale);
    glm::mat4 RotateTrans = glm::eulerAngleXYZ(glm::radians(m_rotateInfo.x),
                                               glm::radians(m_rotateInfo.y),
                                               glm::radians(m_rotateInfo.z));

    glm::mat4 TranslationTrans = glm::translate(glm::mat4(1.0f), m_worldPos);
    m_Wtransformation = TranslationTrans * RotateTrans * ScaleTrans;
    return m_Wtransformation;
}

const glm::mat4 &Pipeline::GetViewTrans()
{
    m_Vtransformation = glm::lookAt(m_camera.Pos,
                                    m_camera.Pos + m_camera.Target,
                                    m_camera.Up);
    return m_Vtransformation;
}

const glm::mat4 &Pipeline::GetWVPTrans()
{
    GetWorldTrans();
    GetVPTrans();

    m_WVPtransformation = m_VPtransformation * m_Wtransformation;
    return m_WVPtransformation;
}

const glm::mat4 &Pipeline::GetWVOrthoPTrans()
{
    GetWorldTrans();
    GetViewTrans();

    glm::mat4 p = glm::ortho(m_orthoProjInfo.l, m_orthoProjInfo.r,
                             m_orthoProjInfo.b, m_orthoProjInfo.t,
                             m_orthoProjInfo.n, m_orthoProjInfo.f);

    m_WVPtransformation = p * m_Vtransformation * m_Wtransformation;
    return m_WVPtransformation;
}

const glm::mat4 &Pipeline::GetWVTrans()
{
    GetWorldTrans();
    GetViewTrans();

    m_WVtransformation = m_Vtransformation * m_Wtransformation;
    return m_WVtransformation;
}

const glm::mat4 &Pipeline::GetWPTrans()
{
    GetWorldTrans();

    glm::mat4 PersProjTrans = glm::perspectiveFov(glm::radians(m_persProjInfo.FOV),
                                                  m_persProjInfo.Width,
                                                  m_persProjInfo.Height,
                                                  m_persProjInfo.zNear,
                                                  m_persProjInfo.zFar);

    m_WPtransformation = PersProjTrans * m_Wtransformation;
    return m_WPtransformation;
}
