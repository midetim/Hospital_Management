# Build context: repo root (Hospital_Management/)
# docker build -f services/FrontEnd_GUI/Dockerfile .

FROM python:3.11-slim

WORKDIR /app

# Copy generated proto Python files (needed for gRPC stubs)
COPY xcode-build/generated_python/ xcode-build/generated_python/

# Copy Python gRPC client files
COPY common/clients/ common/clients/

# Copy frontend service files
COPY services/FrontEnd_GUI/ services/FrontEnd_GUI/

# Install Python dependencies
RUN pip install --no-cache-dir -r services/FrontEnd_GUI/requirements.txt

WORKDIR /app/services/FrontEnd_GUI

EXPOSE 8920

CMD ["uvicorn", "api_translator:app", "--host", "0.0.0.0", "--port", "8920"]
