import docker
import argparse
import sys
import json
from typing import Literal, Union
from dataclasses import dataclass, asdict
from prettytable import PrettyTable
import os
import tempfile
import tarfile
import yaml


@dataclass
class ContainerInfo:
    id: str
    name: str
    status: str
    image: str
    ports: dict
    cpu_usage: float
    memory_usage: float


class DockerManager:
    def __init__(self):
        self.client = docker.from_env()

    def list_containers(self, all: bool = False) -> list[ContainerInfo]:
        containers = self.client.containers.list(all=all)
        container_info = []
        for c in containers:
            stats = c.stats(stream=False)
            cpu_usage = self._calculate_cpu_percent(stats)
            memory_usage = self._calculate_memory_usage(stats)
            container_info.append(ContainerInfo(
                c.id, c.name, c.status,
                c.image.tags[0] if c.image.tags else "None",
                c.ports, cpu_usage, memory_usage
            ))
        return container_info

    def _calculate_cpu_percent(self, stats):
        cpu_delta = stats["cpu_stats"]["cpu_usage"]["total_usage"] - \
            stats["precpu_stats"]["cpu_usage"]["total_usage"]
        system_delta = stats["cpu_stats"]["system_cpu_usage"] - \
            stats["precpu_stats"]["system_cpu_usage"]
        if system_delta > 0.0:
            return (cpu_delta / system_delta) * 100.0
        return 0.0

    def _calculate_memory_usage(self, stats):
        return stats["memory_stats"]["usage"] / stats["memory_stats"]["limit"] * 100.0

    def manage_container(self, action: Literal["start", "stop", "restart", "remove", "pause", "unpause"], container_id: str) -> str:
        try:
            container = self.client.containers.get(container_id)
            getattr(container, action)()
            return f"Container {container_id} {action}ed"
        except docker.errors.NotFound:
            return f"Container {container_id} not found"

    def create_container(self, image: str, name: str, ports: dict = None, volumes: list = None, environment: dict = None) -> Union[str, ContainerInfo]:
        try:
            container = self.client.containers.run(
                image, name=name, detach=True,
                ports=ports, volumes=volumes,
                environment=environment
            )
            stats = container.stats(stream=False)
            cpu_usage = self._calculate_cpu_percent(stats)
            memory_usage = self._calculate_memory_usage(stats)
            return ContainerInfo(
                container.id, container.name, container.status,
                container.image.tags[0] if container.image.tags else "None",
                container.ports, cpu_usage, memory_usage
            )
        except docker.errors.ImageNotFound:
            return f"Image {image} not found"

    def pull_image(self, image: str) -> str:
        try:
            self.client.images.pull(image)
            return f"Image {image} pulled successfully"
        except docker.errors.ImageNotFound:
            return f"Image {image} not found"

    def list_images(self) -> list[str]:
        return [image.tags[0] for image in self.client.images.list() if image.tags]

    def get_container_logs(self, container_id: str, lines: int = 50) -> str:
        try:
            container = self.client.containers.get(container_id)
            return container.logs(tail=lines).decode('utf-8')
        except docker.errors.NotFound:
            return f"Container {container_id} not found"

    def exec_command(self, container_id: str, cmd: str) -> str:
        try:
            container = self.client.containers.get(container_id)
            exit_code, output = container.exec_run(cmd)
            return f"Exit Code: {exit_code}\nOutput:\n{output.decode('utf-8')}"
        except docker.errors.NotFound:
            return f"Container {container_id} not found"

    def get_container_stats(self, container_id: str) -> Union[str, dict]:
        try:
            container = self.client.containers.get(container_id)
            stats = container.stats(stream=False)
            return {
                "CPU Usage": f"{self._calculate_cpu_percent(stats):.2f}%",
                "Memory Usage": f"{self._calculate_memory_usage(stats):.2f}%",
                "Network I/O": f"In: {stats['networks']['eth0']['rx_bytes']/1024/1024:.2f}MB, Out: {stats['networks']['eth0']['tx_bytes']/1024/1024:.2f}MB",
                "Block I/O": f"In: {stats['blkio_stats']['io_service_bytes_recursive'][0]['value']/1024/1024:.2f}MB, Out: {stats['blkio_stats']['io_service_bytes_recursive'][1]['value']/1024/1024:.2f}MB"
            }
        except docker.errors.NotFound:
            return f"Container {container_id} not found"

    def copy_to_container(self, container_id: str, src: str, dest: str) -> str:
        try:
            container = self.client.containers.get(container_id)
            with tempfile.NamedTemporaryFile() as tmp:
                with tarfile.open(tmp.name, "w:gz") as tar:
                    tar.add(src, arcname=os.path.basename(src))
                container.put_archive(os.path.dirname(dest), tmp.read())
            return f"File {src} copied to container {container_id} at {dest}"
        except docker.errors.NotFound:
            return f"Container {container_id} not found"
        except FileNotFoundError:
            return f"Source file {src} not found"

    def copy_from_container(self, container_id: str, src: str, dest: str) -> str:
        try:
            container = self.client.containers.get(container_id)
            bits, stat = container.get_archive(src)
            with tempfile.NamedTemporaryFile() as tmp:
                for chunk in bits:
                    tmp.write(chunk)
                tmp.seek(0)
                with tarfile.open(fileobj=tmp) as tar:
                    tar.extractall(path=dest)
            return f"File {src} copied from container {container_id} to {dest}"
        except docker.errors.NotFound:
            return f"Container {container_id} not found"
        except KeyError:
            return f"Source file {src} not found in container"

    def export_container(self, container_id: str, output_path: str) -> str:
        try:
            container = self.client.containers.get(container_id)
            with open(output_path, 'wb') as f:
                for chunk in container.export():
                    f.write(chunk)
            return f"Container {container_id} exported to {output_path}"
        except docker.errors.NotFound:
            return f"Container {container_id} not found"

    def import_image(self, image_path: str, repository: str, tag: str) -> str:
        try:
            with open(image_path, 'rb') as f:
                image = self.client.images.import_image(
                    f, repository=repository, tag=tag)
            return f"Image imported as {repository}:{tag}"
        except FileNotFoundError:
            return f"Image file {image_path} not found"

    def build_image(self, dockerfile_path: str, tag: str) -> str:
        try:
            image, logs = self.client.images.build(path=os.path.dirname(
                dockerfile_path), dockerfile=os.path.basename(dockerfile_path), tag=tag)
            return f"Image built successfully with tag {tag}"
        except docker.errors.BuildError as e:
            return f"Error building image: {str(e)}"

    def compose_up(self, compose_file: str) -> str:
        try:
            project = self.client.compose.project.from_config(
                project_name="myproject", config_files=[compose_file])
            project.up()
            return f"Docker Compose services started from {compose_file}"
        except docker.errors.APIError as e:
            return f"Error starting Docker Compose services: {str(e)}"

    def compose_down(self, compose_file: str) -> str:
        try:
            project = self.client.compose.project.from_config(
                project_name="myproject", config_files=[compose_file])
            project.down()
            return f"Docker Compose services stopped from {compose_file}"
        except docker.errors.APIError as e:
            return f"Error stopping Docker Compose services: {str(e)}"


def print_table(data, headers):
    table = PrettyTable()
    table.field_names = headers
    for row in data:
        table.add_row(row)
    print(table)


def parse_key_value_pairs(s: str) -> dict:
    if not s:
        return {}
    return dict(item.split("=") for item in s.split(","))


def main():
    parser = argparse.ArgumentParser(
        description="Comprehensive Docker Management CLI Tool")
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # List containers
    list_parser = subparsers.add_parser("list", help="List containers")
    list_parser.add_argument("--all", action="store_true",
                             help="Show all containers (default shows just running)")
    list_parser.add_argument(
        "--format", choices=["table", "json"], default="table", help="Output format")

    # Manage container
    manage_parser = subparsers.add_parser("manage", help="Manage container")
    manage_parser.add_argument("action", choices=[
                               "start", "stop", "restart", "remove", "pause", "unpause"], help="Action to perform")
    manage_parser.add_argument("container_id", help="Container ID")

    # Create container
    create_parser = subparsers.add_parser(
        "create", help="Create a new container")
    create_parser.add_argument("image", help="Image name")
    create_parser.add_argument("name", help="Container name")
    create_parser.add_argument(
        "--ports", help="Port mappings (e.g., '8080:80,9000:9000')")
    create_parser.add_argument(
        "--volumes", help="Volume mappings (e.g., '/host/path:/container/path')")
    create_parser.add_argument(
        "--env", help="Environment variables (e.g., 'KEY1=VALUE1,KEY2=VALUE2')")

    # Pull image
    pull_parser = subparsers.add_parser("pull", help="Pull an image")
    pull_parser.add_argument("image", help="Image name")

    # List images
    subparsers.add_parser("images", help="List images")

    # View container logs
    logs_parser = subparsers.add_parser("logs", help="View container logs")
    logs_parser.add_argument("container_id", help="Container ID")
    logs_parser.add_argument(
        "--lines", type=int, default=50, help="Number of lines to display")

    # Execute command in container
    exec_parser = subparsers.add_parser(
        "exec", help="Execute command in container")
    exec_parser.add_argument("container_id", help="Container ID")
    exec_parser.add_argument("command", help="Command to execute")

    # View container statistics
    stats_parser = subparsers.add_parser(
        "stats", help="View container statistics")
    stats_parser.add_argument("container_id", help="Container ID")

    # Copy file to container
    cp_to_parser = subparsers.add_parser(
        "cp-to", help="Copy file to container")
    cp_to_parser.add_argument("container_id", help="Container ID")
    cp_to_parser.add_argument("src", help="Source file path")
    cp_to_parser.add_argument("dest", help="Destination path in container")

    # Copy file from container
    cp_from_parser = subparsers.add_parser(
        "cp-from", help="Copy file from container")
    cp_from_parser.add_argument("container_id", help="Container ID")
    cp_from_parser.add_argument("src", help="Source file path in container")
    cp_from_parser.add_argument("dest", help="Destination path on host")

    # Export container
    export_parser = subparsers.add_parser(
        "export", help="Export container to a tar archive")
    export_parser.add_argument("container_id", help="Container ID")
    export_parser.add_argument("output", help="Output file path")

    # Import image
    import_parser = subparsers.add_parser(
        "import", help="Import an image from a tar archive")
    import_parser.add_argument("image_path", help="Path to the tar archive")
    import_parser.add_argument(
        "repository", help="Repository name for the image")
    import_parser.add_argument("tag", help="Tag for the image")

    # Build image
    build_parser = subparsers.add_parser(
        "build", help="Build an image from a Dockerfile")
    build_parser.add_argument("dockerfile", help="Path to the Dockerfile")
    build_parser.add_argument("tag", help="Tag for the image")

    # Docker Compose up
    compose_up_parser = subparsers.add_parser(
        "compose-up", help="Start services defined in a Docker Compose file")
    compose_up_parser.add_argument(
        "compose_file", help="Path to the Docker Compose file")

    # Docker Compose down
    compose_down_parser = subparsers.add_parser(
        "compose-down", help="Stop services defined in a Docker Compose file")
    compose_down_parser.add_argument(
        "compose_file", help="Path to the Docker Compose file")

    args = parser.parse_args()

    manager = DockerManager()

    try:
        if args.command == "list":
            containers = manager.list_containers(all=args.all)
            if args.format == "json":
                print(json.dumps([asdict(c) for c in containers], indent=2))
            else:
                data = [[c.id[:12], c.name, c.status, c.image,
                         f"{c.cpu_usage:.2f}%", f"{c.memory_usage:.2f}%"] for c in containers]
                headers = ["ID", "Name", "Status",
                           "Image", "CPU Usage", "Memory Usage"]
                print_table(data, headers)

        elif args.command == "manage":
            result = manager.manage_container(args.action, args.container_id)
            print(result)

        elif args.command == "create":
            ports = parse_key_value_pairs(args.ports)
            volumes = args.volumes.split(",") if args.volumes else None
            env = parse_key_value_pairs(args.env)
            result = manager.create_container(
                args.image, args.name, ports, volumes, env)
            if isinstance(result, ContainerInfo):
                print(
                    f"Container created - ID: {result.id}, Name: {result.name}, Status: {result.status}")
            else:
                print(result)

        elif args.command == "pull":
            result = manager.pull_image(args.image)
            print(result)

        elif args.command == "images":
            images = manager.list_images()
            print("Available images:")
            for image in images:
                print(image)

        elif args.command == "logs":
            logs = manager.get_container_logs(args.container_id, args.lines)
            print(f"Logs for container {args.container_id}:")
            print(logs)

        elif args.command == "exec":
            result = manager.exec_command(args.container_id, args.command)
            print(result)

        elif args.command == "stats":
            stats = manager.get_container_stats(args.container_id)
            if isinstance(stats, dict):
                for key, value in stats.items():
                    print(f"{key}: {value}")
            else:
                print(stats)

        elif args.command == "cp-to":
            result = manager.copy_to_container(
                args.container_id, args.src, args.dest)
            print(result)

        elif args.command == "cp-from":
            result = manager.copy_from_container(
                args.container_id, args.src, args.dest)
            print(result)

        elif args.command == "export":
            result = manager.export_container(args.container_id, args.output)
            print(result)

        elif args.command == "import":
            result = manager.import_image(
                args.image_path, args.repository, args.tag)
            print(result)

        elif args.command == "build":
            result = manager.build_image(args.dockerfile, args.tag)
            print(result)

        elif args.command == "compose-up":
            result = manager.compose_up(args.compose_file)
            print(result)

        elif args.command == "compose-down":
            result = manager.compose_down(args.compose_file)
            print(result)

    except docker.errors.APIError as e:
        print(f"Docker API Error: {str(e)}")
    except Exception as e:
        print(f"An unexpected error occurred: {str(e)}")


if __name__ == "__main__":
    main()
