////////////////////////////////////////////////////////////
/// \mainpage
///
/// \section welcome Welcome
/// Welcome to the official robot2D documentation. Here you will find a
/// view of the robot2D classes and functions. <br/>
/// You can find sources at <a href="https://github.com/Enziferum/robot2D"> github/robot2D </a>.
///
/// \section example Short example
/// Here is a short example, to show you how simple it is to use robot2D:
///
/// \code
///
/// #include <robot2D/robot2D.hpp>
///
/// int main()
/// {
///     // Create the main window
///     robot2D::RenderWindow window(robot2D::vec2u(800, 600), "robot2D Window");
///
///     // Load a sprite to display
///     robot2D::Texture texture;
///     if (!texture.loadFromFile("cute_image.jpg"))
///         return EXIT_FAILURE;
///     robot2D::Sprite sprite(texture);
///
///     // Create a graphical text to display
///     robot2D::Font font;
///     if (!font.loadFromFile("arial.ttf"))
///         return EXIT_FAILURE;
///     robot2D::Text text("Hello robot2D", font, 50);
///
///
///
///     // Start the game loop
///     while (window.isOpen())
///     {
///         // Process events
///         robot2D::Event event;
///         while (window.pollEvent(event))
///         {
///             // Close window: exit
///             if (event.type == robot2D::Event::Closed)
///                 window.close();
///         }
///
///         // Clear screen
///         window.clear();
///
///         // Draw the sprite
///         window.draw(sprite);
///
///         // Draw the string
///         window.draw(text);
///
///         // Update the window
///         window.display();
///     }
///
///     return EXIT_SUCCESS;
/// }
/// \endcode
////////////////////////////////////////////////////////////