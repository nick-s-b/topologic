/**\file
 * \brief Render context
 *
 * The individual renderers provided by libefgy have very similar interfaces,
 * but there are some minor differences that need to be mangled around a bit
 * so that topologic can just substitute one renderer for another. The classes
 * in this file provide that mangling.
 *
 * \copyright
 * Copyright (c) 2012-2014, Topologic Project Members
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \see Project Documentation: http://ef.gy/documentation/topologic
 * \see Project Source Code: https://github.com/ef-gy/topologic
 */

#if !defined(TOPOLOGIC_RENDER_H)
#define TOPOLOGIC_RENDER_H

#include <ef.gy/render-svg.h>
#if !defined(NO_OPENGL)
#include <ef.gy/render-opengl.h>
#endif

namespace topologic
{
    /**\brief Cartesian dimension shorthands
     *
     * As you'll probably remember form high school, it's customary to label the
     * first three dimensions in a euclidian coordinate space "x", "y" and "z".
     * If you've had a poke at OpenGL or two, you'll also remember that the
     * fourth coordinate is often called "w". But what about the remaining "n"
     * dimensions? Well, we could just use "dimension-N", but that's kind of
     * dull, and since Topologic could, in theory, use a lot more dimensions
     * than 4, this array here defines how these dimensions are labelled - e.g.
     * in XML metadata fragments.
     */
    static const char cartesianDimensions[] = "xyzwvutsrqponmlkjihgfedcbaZYXWVUTSRQPONMLKJIHGFEDCBA";

    template<typename Q, unsigned int d> class state;

    /**\brief Templates related to Topologic's rendering process
     *
     * This namespace encompasses all of the templates related to topologic's
     * actual rendering process, as opposed to state management or setup tasks.
     */
    namespace render
    {
        /**\brief Model metadata
         *
         * Holds all the common model metadata that is needed to identify a
         * model type.
         */
        class metadata
        {
            public:
                /**\brief Construct with model metadata
                 *
                 * Sets the basic metadata for a model.
                 */
                metadata(unsigned int pDepth = 0,
                         unsigned int pRenderDepth = 0,
                         const char *pID = "none",
                         const char *pFormatID = "default")
                    : depth(pDepth), renderDepth(pRenderDepth),
                      id(pID), formatID(pFormatID),
                      update(true)
                    {}
                
                /**\brief Query model depth
                 *
                 * Used to access the model depth; this is typically a template
                 * parameter.
                 *
                 * \returns The model depth; expect values like "2" for a
                 *          square, "3" for a cube, etc.
                 */
                const unsigned int depth;
                
                /**\brief Query render depth
                 *
                 * Used to access the depth that the renderer has been
                 * initialised to.
                 *
                 * \returns The model renderer's depth; expect this value to be
                 *          greater than or equal to the model's depth.
                 */
                const unsigned int renderDepth;

                /**\brief Query model name
                 *
                 * Used to obtain a short, descriptive name of a model. This
                 * name is also used when instantiating the model with a
                 * factory.
                 *
                 * \returns A C-style, 0-terminated string containing the name
                 *          of the model. This should never return a 0-pointer.
                 */
                const char *id;

                /**\brief Query extended model name
                 *
                 * This returns a string of the form "depth()-id()", e.g.
                 * "4-cube" for a 4D model with the id "cube".
                 *
                 * \returns A C++ std::string containing the model's name.
                 */
                std::string name (void) const
                {
                    std::stringstream rv;
                    rv << metadata::depth << "-" << id;
                    return rv.str();
                }

                /**\brief Query vector format ID
                 *
                 * Used to obtain a string that identifies the vector format
                 * currently used by the model.
                 *
                 * \returns Vector format ID string.
                 */
                const char *formatID;

                /**\brief Force internal update
                 *
                 * This tells a renderer that it should do a full redraw,
                 * because you changed some parameters that it may have cached.
                 */
                bool update;
        };

        /**\brief Base class for a model renderer
         *
         * The primary purpose of this class is to force certain parts of a
         * model renderer's interface to be virtual.
         *
         * \tparam isVirtual Whether the derived class should contain virtual
         *                   methods.
         */
        template<bool isVirtual = false>
        class base : public metadata
        {
            public:
                /**\brief Construct with model metadata
                 *
                 * Sets the basic metadata for a model.
                 */
                base(unsigned int pDepth = 0,
                     unsigned int pRenderDepth = 0,
                     const char *pID = "none",
                     const char *pFormatID = "default")
                    : metadata(pDepth, pRenderDepth, pID, pFormatID)
                    {}

                /**\brief Virtual destructor
                 *
                 * Generally necessary for virtual classes; stubbed to be a
                 * trivial destructor.
                 */
                virtual ~base(void) {}

                /**\brief Render to SVG
                 *
                 * This is a wrapper for libefgy's SVG renderer, augmented with
                 * some code to write out model parameters and use Topologic's
                 * state object to handle these parameters.
                 *
                 * \param[in] output       The stream to write to.
                 * \param[in] updateMatrix Whether to update the projection
                 *                         matrices.
                 *
                 * \returns 'true' upon success.
                 */
                virtual bool svg (std::ostream &output,
                                  bool updateMatrix = false);

#if !defined (NO_OPENGL)
                /**\brief Render to OpenGL context
                 *
                 * This is a wrapper for libefgy's OpenGL renderer.
                 *
                 * \param[in] updateMatrix Whether to update the projection
                 *                         matrices.
                 *
                 * \returns 'true' upon success.
                 */
                virtual bool opengl (bool updateMatrix = false) = 0;
#endif
        };

        /**\brief Non-virtual model renderer base class
         *
         * This is simply an empty class, which allows a model renderer to be
         * non-virtual, which in turn would probably be handy in certain
         * situations where only a very select few models will be used and it
         * would be a good idea to highly optimise the renderers for these
         * models; by cutting down on the number of virtual functions, the
         * compiler should be able to provide slightly better code.
         */
        template<>
        class base<false> : public metadata
        {
            public:
                /**\brief Construct with model metadata
                 *
                 * Sets the basic metadata for a model.
                 */
                base(unsigned int pDepth = 0,
                     unsigned int pRenderDepth = 0,
                     const char *pID = "none",
                     const char *pFormatID = "default")
                    : metadata(pDepth, pRenderDepth, pID, pFormatID)
                    {}
        };

        /**\brief Renderer base class with default methods
         *
         * This template provides some of the basic functionality shared
         * between distinct model renderers which aren't being provided by the
         * renderers in libefgy, or which only need to be passed along.
         *
         * \tparam Q  Base data type for calculations
         * \tparam d  Model depth; typically has to be <= the render depth
         * \tparam T  Model template; use things like efgy::geometry::cube
         * \tparam isVirtual Whether the class should contain the virtual
         *                   functions defined in renderer<true>. It'll contain
         *                   the actual functions in there either way, this
         *                   just determines if they should make the class a
         *                   virtual class.
         * \tparam format    The vector format to use.
         */
        template<typename Q, unsigned int d, template <class,unsigned int> class T, bool isVirtual, typename format>
        class wrapper : public base<isVirtual>
        {
            public:
                /**\brief Model type
                 *
                 * Alias of the model type that this class represents with all
                 * the template parameters filled in.
                 */
                typedef T<Q,d> modelType;

                /* \brief Model render depth
                 *
                 * This is the maximum depth for your models and also specifies
                 * the maximum depth of any transformations you can apply.
                 */
                static constexpr const unsigned int rd = modelType::renderDepth;

                /**\brief Global state type
                 *
                 * Alias of the global state type that this class uses to
                 * gather information to render a model with all the template
                 * parameters filled in.
                 */
                typedef state<Q,rd> stateType;

                /**\brief Construct with global state and renderer
                 *
                 * Sets the object up with a global state object and an
                 * appropriate renderer instance. The parameters and export
                 * multipliers passed to the model are the default ones
                 * provided by the global state object.
                 *
                 * \param[in,out] pState  The global topologic::state instance
                 * \param[in]     pFormat The vector format tag to use
                 */
                wrapper(stateType &pState, const format &pFormat)
                    : gState(pState),
                      object(gState.parameter, pFormat),
                      base<isVirtual>(d, rd, modelType::id(), modelType::format::id())
                    {}

                /**\brief Construct with global state, renderer and parameters
                 *
                 * Like the two-parameter constructor, but provides a custom
                 * parameter object.
                 *
                 * \param[in,out] pState     The global topologic::state
                 *                           instance
                 * \param[in]     pParameter The parameter instance to use
                 * \param[in]     pFormat    The vector format tag to use
                 */
                wrapper(stateType &pState,
                       const efgy::geometry::parameters<Q> &pParameter,
                       const format &pFormat)
                    : gState(pState),
                      object(pParameter, pFormat),
                      base<isVirtual>(d, rd, modelType::id(), modelType::format::id())
                    {}

                bool svg (std::ostream &output,
                          bool updateMatrix = false)
                {
                    if (metadata::update)
                    {
                        object.calculateObject();
                        metadata::update = false;
                    }

                    if (updateMatrix)
                    {
                        gState.width  = 3;
                        gState.height = 3;
                        gState.updateMatrix();
                    }
                    
                    gState.svg.frameStart();
                    
                    output
                        <<  "<?xml version='1.0' encoding='utf-8'?>"
                            "<svg xmlns='http://www.w3.org/2000/svg'"
                            " xmlns:xlink='http://www.w3.org/1999/xlink'"
                            " version='1.1' width='100%' height='100%' viewBox='-1.2 -1.2 2.4 2.4'>"
                            "<title>" + metadata::name() + "</title>"
                            "<metadata xmlns:t='http://ef.gy/2012/topologic'>"
                        <<  efgy::xml::tag() << gState;
                    output
                        <<  "</metadata>"
                            "<style type='text/css'>svg { background: rgba(" << double(gState.background.red)*100. << "%," <<double(gState.background.green)*100. << "%," << double(gState.background.blue)*100. << "%," << double(gState.background.alpha) << "); }"
                            " path { stroke-width: 0.002; stroke: rgba(" << double(gState.wireframe.red)*100. << "%," << double(gState.wireframe.green)*100. << "%," << double(gState.wireframe.blue)*100. << "%," << double(gState.wireframe.alpha) << ");"
                            " fill: rgba(" << double(gState.surface.red)*100. << "%," << double(gState.surface.green)*100. << "%," << double(gState.surface.blue)*100. << "%," << double(gState.surface.alpha) << "); }</style>";
                    if (gState.surface.alpha > Q(0.))
                    {
                        output << gState.svg << object;
                    }
                    output << "</svg>\n";
                    
                    gState.svg.frameEnd();
                    
                    return true;
                }

#if !defined (NO_OPENGL)
                bool opengl (bool updateMatrix = false)
                {
                    if (metadata::update)
                    {
                        gState.opengl.prepared = false;
                        object.calculateObject();
                        metadata::update = false;
                    }

                    if (updateMatrix)
                    {
                        gState.updateMatrix();
                    }

                    gState.opengl.fractalFlameColouring = gState.fractalFlameColouring;
                    gState.opengl.width  = gState.width;
                    gState.opengl.height = gState.height;
                    
                    if (!gState.fractalFlameColouring)
                    {
                        glClearColor
                            (gState.background.red, gState.background.green,
                             gState.background.blue, gState.background.alpha);
                    }
                    
                    gState.opengl.frameStart();
                    
                    gState.opengl.wireframeColour = gState.wireframe;
                    gState.opengl.surfaceColour   = gState.surface;

                    if (!gState.opengl.prepared)
                    {
                        std::cerr << gState.opengl << object;
                    }
                    
                    gState.opengl.clear();
                    gState.opengl.frameEnd();
                    
                    return true;
                }
#endif

            protected:
                /**\brief Global state object
                 *
                 * A reference to the global state object, which was passed to
                 * the constructor as 'pState'. This is necessary to keep track
                 * of updated global settings.
                 */
                stateType &gState;

                /**\brief Intrinsic object instance
                 *
                 * Contains the instance of the model that this renderer is
                 * trying to create a representation of.
                 */
                modelType object;
        };
    };
};

#endif
