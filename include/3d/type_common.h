#ifndef __TYPE_COMMON__
#define __TYPE_COMMON__

/**
 * @author: Methusael Murmu
 */
 
/* Define common types for convenience */

#include <external/glm/vec2.hpp>
#include <external/glm/vec3.hpp>
#include <external/glm/mat4x4.hpp>

typedef glm::vec2       t_v2;
typedef glm::vec3       t_v3;
typedef glm::mat4       t_m4;

typedef const t_v2&     t_rcv2;
typedef const t_v3&     t_rcv3;
typedef const t_m4&     t_rcm4;

#endif
