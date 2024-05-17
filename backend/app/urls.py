# backend/app/urls.py

from django.contrib import admin
from django.urls import include, path

urlpatterns = [
    path('admin/', admin.site.urls),
    path('analysis/', include('analysis.urls')),
    path('game/', include('game.urls')),
    path('user/', include('user.urls')),
]
