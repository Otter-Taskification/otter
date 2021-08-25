from setuptools import find_packages, setup

setup(name="otter",
      version="0.1",
      description="Otter post-processing tool",
      author="Adam Tuft",
      author_email='adam.s.tuft@gmail.com',
      platforms=["linux"],
      license="https://github.com/adamtuft/otter/blob/main/LICENSE",
      url="https://github.com/adamtuft/otter",
      packages=find_packages(),
      install_requires=[
            'python-igraph==0.9.1'
      ],
      dependency_links=['https://perftools.pages.jsc.fz-juelich.de/cicd/otf2/']
      )
