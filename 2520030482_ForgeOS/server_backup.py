from http.server import HTTPServer, BaseHTTPRequestHandler
import subprocess
import json
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

class Server(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path in ["/", "/index.html"]:
            try:
                with open(os.path.join(BASE_DIR, "web", "index.html"), "rb") as f:
                    content = f.read()

                self.send_response(200)
                self.send_header("Content-Type", "text/html")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)

            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(str(e).encode())
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path != "/simulate":
            self.send_response(404)
            self.end_headers()
            return

        try:
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length)
            data = json.loads(body.decode())

            processes = data.get("processes", [])
            algorithm = data.get("algorithm", "FCFS")
            quantum = data.get("quantum", 2)

            input_data = str(len(processes)) + "\n"

            for p in processes:
                input_data += (
                    f"{p.get('pid', '')} "
                    f"{p.get('arrivalTime', p.get('arrival', 0))} "
                    f"{p.get('burstTime', p.get('burst', 0))} "
                    f"{p.get('priority', 0)}\n"
                )

            choices = {
                "FCFS": "1",
                "SJF": "2",
                "Priority Scheduling": "3",
                "Round Robin": "4"
            }

            choice = choices.get(algorithm, "1")
            input_data += choice + "\n"

            if choice == "4":
                input_data += str(quantum) + "\n"

            result = subprocess.run(
                [os.path.join(BASE_DIR, "cpu_simulator")],
                input=input_data,
                text=True,
                capture_output=True,
                cwd=BASE_DIR
            )

            response = {
                "success": result.returncode == 0,
                "output": result.stdout,
                "error": result.stderr if result.returncode != 0 else ""
            }

        except Exception as e:
            response = {
                "success": False,
                "error": str(e)
            }

        result = json.dumps(response).encode()

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(result)))
        self.end_headers()
        self.wfile.write(result)


server = HTTPServer(("0.0.0.0", 8000), Server)

print("Server started successfully!")
print("Open: http://wsl.localhost:8000")

server.serve_forever()
