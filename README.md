
# Image Request Server

This project allows a user to connect to a server, request an image using a keyword, and receive that image. The server fetches the image from Pixabay via its API and sends it back to the client.

## Requirements

- **Pixabay API Key**: You need a Pixabay API key to use their image search functionality. Create an account at [Pixabay](https://pixabay.com/api/docs/) and get your API key.
- **Libraries**: Make sure you have the following libraries installed:
  - `libcurl`: For making HTTP requests to the Pixabay API.
  - `json-c`: For parsing JSON responses from the Pixabay API.

### Installation

1. **Install dependencies**:
   On Linux, you can install `libcurl` and `json-c` using the following commands:
   
   ```bash
   sudo apt-get install libcurl4-openssl-dev
   sudo apt-get install libjson-c-dev
   ```

2. **Clone the repository**:
   ```bash
   git clone https://github.com/your-repo/image-request-server.git
   ```

3. **Compile the client and server**:
   Use a C compiler like `gcc` to compile the client and server files. For example:
   ```bash
   gcc -o server server.c -lcurl -ljson-c
   gcc -o client client.c
   ```

### Usage

1. **Start the server**:
   Run the server on your machine. It will listen for incoming connections.
   
   ```bash
   ./server
   ```

2. **Run the client**:
   After starting the server, run the client, which will connect to the server and prompt you for a keyword to search for an image.
   
   ```bash
   ./client
   ```

3. **Enter a keyword**:
   The client will ask for a keyword. Type a search term, and the server will fetch the relevant image from Pixabay and send it back to the client.

4. **Receive the image**:
   The image will be saved in the current directory of the client as `image_recue.jpg`.

### Server Code Overview

- The server listens for incoming client connections.
- Upon receiving a connection, it reads the keyword sent by the client.
- It makes an API call to Pixabay using the provided keyword.
- The server downloads the image and sends it back to the client.

### Client Code Overview

- The client connects to the server and prompts the user for a keyword.
- It sends the keyword to the server and waits to receive an image.
- Once the image is received, it is saved to disk as `image_recue.jpg`.

### Configuration

- **Pixabay API Key**: Replace the placeholder API key in the server code with your own:
  
  ```c
  #define PIXABAY_API_KEY "your-api-key-here"
  ```

- **Port**: By default, the server listens on port `5094`. You can change this in the `#define LISTENING_PORT` directive in both the client and server code.

### License

This project is licensed under the MIT License.
