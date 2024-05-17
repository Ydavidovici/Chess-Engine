# backend/user/views.py

from django.contrib.auth.decorators import login_required
from django.http import JsonResponse

@login_required
def user_profile(request):
    user = request.user
    return JsonResponse({"username": user.username, "email": user.email})
