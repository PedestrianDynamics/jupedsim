import pytest
from driver.environment import Environment


@pytest.fixture
def env():
    return Environment()
