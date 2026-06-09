from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from datetime import datetime, timedelta
from jose import jwt
import os

router = APIRouter()

SECRET_KEY = os.getenv("SECRET_KEY", "CHANGE_THIS_SECRET_KEY_IN_PRODUCTION_ENV")
ALGORITHM = "HS256"

ADMIN_USER = os.getenv("ADMIN_USERNAME", "admin")
ADMIN_PASS = os.getenv("ADMIN_PASSWORD", "changeme")


class LoginRequest(BaseModel):
    username: str
    password: str


def create_token(data: dict):
    expire = datetime.utcnow() + timedelta(hours=72)
    return jwt.encode({**data, "exp": expire}, SECRET_KEY, algorithm=ALGORITHM)


@router.post("/login")
async def login(req: LoginRequest):

    if req.username != ADMIN_USER or req.password != ADMIN_PASS:
        raise HTTPException(status_code=401, detail="Invalid credentials")

    token = create_token({"sub": req.username})

    return {
        "access_token": token,
        "token_type": "bearer"
    }