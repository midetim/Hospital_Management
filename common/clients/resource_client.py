import sys
import os

# Add the generated protobuf Python files to the Python path
# This allows us to import *_pb2 and *_pb2_grpc modules
sys.path.insert(
    0,
    os.path.abspath(
        os.path.join(os.path.dirname(__file__), "../../xcode-build/generated_python")
    ),
)

# gRPC library for making remote procedure calls
import grpc

# Import generated protobuf files for Common service
import Common_pb2
import Common_pb2_grpc

# Import generated protobuf files for ResourceManagement service
import ResourceManagement_pb2
import ResourceManagement_pb2_grpc


class ResourceManagementClient:
    def __init__(self, target: str):
        """
        Constructor:
        - Creates a connection (channel) to the gRPC server
        - Initializes stubs for calling RPC methods
        """
        self.target_hostport = target

        # Create communication channel to server (e.g., "localhost:50051")
        self.channel = grpc.insecure_channel(self.target_hostport)

        # Stub = client-side proxy to call RPC functions
        self.stub = ResourceManagement_pb2_grpc.ResourceManagementStub(self.channel)

        # Common service stub (for ping, print, update)
        self.common = Common_pb2_grpc.CommonStub(self.channel)

    # ============================================================
    # 🔹 Common gRPC Methods (shared across all services)
    # ============================================================

    def ping(self, service_name: str) -> bool:
        """
        Checks if the service is alive
        """
        request = Common_pb2.Nothing()
        try:
            self.common.ping(request, metadata=[("service-name", service_name)])
            return True
        except grpc.RpcError as e:
            print("Ping failed:", e)
            return False

    def print_service(self, service_name: str) -> bool:
        """
        Requests the service to print its internal state
        """
        request = Common_pb2.Nothing()
        try:
            self.common.print(request, metadata=[("service-name", service_name)])
            return True
        except grpc.RpcError as e:
            print("Print failed:", e)
            return False

    def update(self, service_name: str) -> bool:
        """
        Requests the service to reload/update its data (e.g., from DB)
        """
        request = Common_pb2.Nothing()
        try:
            self.common.update(request, metadata=[("service-name", service_name)])
            return True
        except grpc.RpcError as e:
            print("Update failed:", e)
            return False

    # ============================================================
    # 🔹 Resource Management Methods (your main task)
    # ============================================================

    def getInfo(self, resource_id: int, service_name: str):
        """
        Retrieves information about a specific resource

        Returns:
            dict with resource details OR None if failed
        """
        # Create request message (only ID needed)
        request = ResourceManagement_pb2.ResourceDTO(
            resource_id=resource_id
        )

        try:
            # Call RPC method
            response = self.stub.GetResourceInformation(
                request,
                metadata=[("service-name", service_name)]
            )

            # Convert protobuf response → Python dictionary
            return {
                "resource_id": response.resource_id,
                "room_id": response.room_id,
                "resource_type": response.resource_type,
                "resource_stock": response.resource_stock
            }

        except grpc.RpcError as e:
            print("getInfo failed:", e)
            return None

    def registerResource(self, resource_type: str, room_id: int, stock: int, service_name: str):
        """
        Registers a new resource in the system
        """
        # Build request object with resource details
        request = ResourceManagement_pb2.ResourceDTO(
            resource_type=resource_type,
            room_id=room_id,
            resource_stock=stock
        )

        try:
            # Call RPC
            response = self.stub.RegisterResource(
                request,
                metadata=[("service-name", service_name)]
            )

            # Return success status
            return response.successful

        except grpc.RpcError as e:
            print("registerResource failed:", e)
            return False

    def deregisterResource(self, resource_id: int, service_name: str):
        """
        Removes a resource from the system
        """
        request = ResourceManagement_pb2.ResourceDTO(
            resource_id=resource_id
        )

        try:
            response = self.stub.DeregisterResource(
                request,
                metadata=[("service-name", service_name)]
            )
            return response.successful

        except grpc.RpcError as e:
            print("deregisterResource failed:", e)
            return False

    def addStock(self, resource_id: int, resource_type: str, amount: int, service_name: str):
        """
        Increases stock quantity for a resource
        """
        request = ResourceManagement_pb2.StockUpdate(
            resource_id=resource_id,
            resource_type=resource_type,
            stock_amount=amount
        )

        try:
            response = self.stub.AddStock(
                request,
                metadata=[("service-name", service_name)]
            )
            return response.successful

        except grpc.RpcError as e:
            print("addStock failed:", e)
            return False

    def useStock(self, resource_id: int, resource_type: str, amount: int, service_name: str):
        """
        Decreases/uses stock for a resource
        """
        request = ResourceManagement_pb2.StockUpdate(
            resource_id=resource_id,
            resource_type=resource_type,
            stock_amount=amount
        )

        try:
            response = self.stub.UseStock(
                request,
                metadata=[("service-name", service_name)]
            )
            return response.successful

        except grpc.RpcError as e:
            print("useStock failed:", e)
            return False