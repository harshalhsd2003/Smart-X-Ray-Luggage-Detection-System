# Local Development Setup

## 1. Create and activate conda environment

```bash
conda create -n xraysim python=3.10
conda activate xraysim
```

## 2. Install dependencies

```bash
pip install -r requirements.txt
```

## 3. Configure environment variables

Copy `.env.example` to `.env` and fill in your values:

```bash
cp .env.example .env
```

## 4. Run the cloud backend

```bash
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

## 5. Run the local scanner (ai-engine)

Navigate to the `ai-engine/` folder and run:

```bash
python gpulocal3.py
```
